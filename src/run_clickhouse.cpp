#include "run_clickhouse.h"

extern pthread_t conn_threads[CONN_THREADS];
extern std::deque<connection_t *> conn_queue;
extern pthread_mutex_t conn_mtx;
extern pthread_cond_t conn_cond_var;

extern std::deque<char *> read_queue;
extern pthread_t read_threads[READ_THREADS];
extern pthread_mutex_t read_mtx;
extern pthread_cond_t read_cond_var;

extern std::deque<std::string> insert_statements;
extern pthread_t write_threads[WRITE_THREADS];
extern pthread_mutex_t write_mtx;
extern pthread_cond_t write_cond_var;

extern pthread_t sync_thread;

namespace Clickhouse {

clickhouse::Client *db =
    (clickhouse::Client *)malloc(sizeof(clickhouse::Client) * WRITE_THREADS);

static void *conn_worker(void *arg) {
  while (true) {
    pthread_mutex_lock(&conn_mtx);
    while (conn_queue.empty()) {
      pthread_cond_wait(&conn_cond_var, &conn_mtx);
    }

    connection_t *conn = conn_queue.front();
    conn_queue.pop_front();
    pthread_mutex_unlock(&conn_mtx);

    char buf[BUFFER_SIZE];
    while (read(conn->client_socket, buf, BUFFER_SIZE) > 0) {
      pthread_mutex_lock(&read_mtx);
      read_queue.push_front(buf);
      pthread_cond_broadcast(&read_cond_var);
      pthread_mutex_unlock(&read_mtx);
    }

    delete conn;
  }

  free(arg);
  pthread_exit(nullptr);
}

static void *read_worker(void *arg) {
  while (true) {
    pthread_mutex_lock(&read_mtx);
    while (read_queue.empty()) {
      pthread_cond_wait(&read_cond_var, &read_mtx);
    }

    char *buf = read_queue.front();
    read_queue.pop_front();
    pthread_mutex_unlock(&read_mtx);

    Log entry = Serializer::serialize(buf);
    if (entry.is_vaild) {
      std::string query = ClickhouseQueryGenerator::create_subquery(entry);
      pthread_mutex_lock(&write_mtx);
      insert_statements.push_front(query);
      pthread_cond_broadcast(&write_cond_var);
      pthread_mutex_unlock(&write_mtx);
    }
  }

  free(arg);
  pthread_exit(nullptr);
}

static void *write_worker(void *arg) {
  while (true) {

    int index = *((int *)arg);

    pthread_mutex_lock(&write_mtx);
    while (insert_statements.size() < SAVE_THRESHOLD) {
      pthread_cond_wait(&write_cond_var, &write_mtx);
    }

    std::deque<std::string> bucket;
    while (!insert_statements.empty()) {
      bucket.push_back(insert_statements.front());
      insert_statements.pop_front();
    }

    pthread_mutex_unlock(&write_mtx);

    std::string query_string = ClickhouseQueryGenerator::create_query(bucket);
    db[index].Execute(query_string);
    info("evenscribe: Save success.\n");
  }
  free(arg);
  pthread_exit(nullptr);
}

static void *sync_worker(void *arg) {
  while (true) {
    sleep(TIME_TO_SAVE);

    pthread_mutex_lock(&write_mtx);

    if (insert_statements.size() == 0) {
      pthread_mutex_unlock(&write_mtx);
      continue;
    }

    std::deque<std::string> bucket;
    while (!insert_statements.empty()) {
      bucket.push_back(insert_statements.front());
      insert_statements.pop_front();
    }

    pthread_mutex_unlock(&write_mtx);

    std::string query_string = ClickhouseQueryGenerator::create_query(bucket);
    db[0].Execute(query_string);
    info("evenscribe: Save success.\n");
  }
}

void run(Config config) {
  for (int i = 0; i < CONN_THREADS; ++i) {
    if (pthread_create(&conn_threads[i], NULL, *conn_worker, NULL) != 0) {
      error("create thread failed");
    }
  }

  for (int i = 0; i < READ_THREADS; ++i) {
    if (pthread_create(&read_threads[i], NULL, *read_worker, NULL) != 0) {
      error("create thread failed");
    }
  }

  for (int i = 0; i < WRITE_THREADS; ++i) {
    // FIXME: this shit right here throws error
    // but cannot be caught for some reason
    // doesn't even print a debug message
    new (&db[i]) clickhouse::Client(
        clickhouse::ClientOptions()
            .SetHost(config.host)
            .SetPort(config.port)
            .SetUser(config.user)
            .SetPassword(config.password)
            .SetSSLOptions(clickhouse::ClientOptions::SSLOptions()));

    int *a = (int *)malloc(sizeof(int));
    *a = i;
    if (pthread_create(&write_threads[i], nullptr, *write_worker, a) != 0) {
      error("create thread failed");
    }
  }

  if (pthread_create(&sync_thread, NULL, *sync_worker, NULL) != 0) {
    error("create thread failed");
  };
}
} // namespace Clickhouse
