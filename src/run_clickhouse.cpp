#include "clickhouse_query_generator.cpp"
#include "config.h"
#include "helper.h"
#include "log.h"
#include "param.h"
#include <clickhouse/client.h>
#include <queue>
#include <unistd.h>

extern pthread_t conn_threads[CONN_THREADS];
extern std::queue<connection_t *> conn_queue;
extern pthread_mutex_t conn_mtx;
extern pthread_cond_t conn_cond_var;
extern bool conn_done;

extern std::queue<std::string> read_queue;
extern pthread_t read_threads[READ_THREADS];
extern pthread_mutex_t read_mtx;
extern pthread_cond_t read_cond_var;
extern bool read_done;

extern std::vector<std::tuple<std::string, time_t>> insert_statements;
extern pthread_t write_threads[WRITE_THREADS];
extern pthread_mutex_t write_mtx;
extern pthread_cond_t write_cond_var;
extern bool write_done;

ClickhouseQueryGenerator clickhouse_query_generator;
clickhouse::Client *db_clickhouse =
    (clickhouse::Client *)malloc(sizeof(clickhouse::Client) * WRITE_THREADS);

void *conn_worker_clickhouse(void *arg) {
  while (true) {
    pthread_mutex_lock(&conn_mtx);
    while (conn_queue.empty() && !conn_done) {
      pthread_cond_wait(&conn_cond_var, &conn_mtx);
    }
    if (conn_done && conn_queue.empty()) {
      pthread_mutex_unlock(&conn_mtx);
      break;
    }

    connection_t *conn = conn_queue.front();
    conn_queue.pop();
    pthread_mutex_unlock(&conn_mtx);

    char buf[BUFFER_SIZE];
    while (read(conn->client_socket, buf, BUFFER_SIZE) > 0) {
      pthread_mutex_lock(&read_mtx);
      read_queue.push(buf);
      pthread_cond_broadcast(&read_cond_var);
      pthread_mutex_unlock(&read_mtx);
    }

    delete conn;
  }

  free(arg);
  pthread_exit(nullptr);
}

void *read_worker_clickhouse(void *arg) {
  while (true) {
    pthread_mutex_lock(&read_mtx);
    while (read_queue.empty() && !read_done) {
      pthread_cond_wait(&read_cond_var, &read_mtx);
    }
    if (read_done && read_queue.empty()) {
      pthread_mutex_unlock(&read_mtx);
      break;
    }

    std::string buf = read_queue.front();
    read_queue.pop();
    pthread_mutex_unlock(&read_mtx);

    Log entry = Serializer::serialize(buf);
    if (entry.is_vaild) {
      std::string query = clickhouse_query_generator.create_subquery(entry);
      pthread_mutex_lock(&write_mtx);
      insert_statements.push_back(std::make_tuple(query, time(NULL)));
      pthread_cond_broadcast(&write_cond_var);
      pthread_mutex_unlock(&write_mtx);
    }
  }

  free(arg);
  pthread_exit(nullptr);
}

void *write_worker_clickhouse(void *arg) {
  while (true) {
    // if (insert_statements.size() > 0) {
    //   bool should_insert =
    //       difftime(time(NULL), std::get<1>(insert_statements[0])) > 10;
    //   if (should_insert) {
    //     std::string query_string = get_query_from_bucket(insert_statements);
    //     try {
    //       db_clickhouse[0].Execute(query_string);
    //     } catch (const std::exception &e) {
    //       warn(e.what());
    //     }
    //     info("Save success auto.\n");
    //     insert_statements.clear();
    //   }
    // }

    int index = *((int *)arg);

    pthread_mutex_lock(&write_mtx);
    while (insert_statements.size() < SAVE_THRESHOLD && !write_done) {
      pthread_cond_wait(&write_cond_var, &write_mtx);
    }

    if (write_done && insert_statements.size() < SAVE_THRESHOLD) {
      pthread_mutex_unlock(&write_mtx);
      break;
    }

    std::vector<std::tuple<std::string, time_t>> thread_copy(
        insert_statements.begin(), insert_statements.end());
    insert_statements.clear();
    pthread_mutex_unlock(&write_mtx);

    std::string query_string =
        clickhouse_query_generator.create_query(thread_copy);
    db_clickhouse[index].Execute(query_string);
    info("Save success.\n");
  }
  free(arg);
  pthread_exit(nullptr);
}

void run_clickhouse(Config config) {
  for (int i = 0; i < CONN_THREADS; ++i) {
    if (pthread_create(&conn_threads[i], NULL, *conn_worker_clickhouse, NULL) !=
        0) {
      error("create thread failed");
    }
  }

  for (int i = 0; i < READ_THREADS; ++i) {
    if (pthread_create(&read_threads[i], NULL, *read_worker_clickhouse, NULL) !=
        0) {
      error("create thread failed");
    }
  }

  for (int i = 0; i < WRITE_THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    // FIXME: this shit right here throws error
    // but cannot be caught for some reason
    // doesn't even print a debug message
    new (&db_clickhouse[i]) clickhouse::Client(
        clickhouse::ClientOptions()
            // .SetSSLOptions(clickhouse::ClientOptions::SSLOptions())
            .SetHost(config.host)
            .SetPort(config.port)
        // .SetUser(config.user)
        // .SetPassword(config.password)
    );
    if (pthread_create(&write_threads[i], nullptr, *write_worker_clickhouse,
                       a) != 0) {
      error("create thread failed");
    }
  }
}
