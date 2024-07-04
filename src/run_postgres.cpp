#include "config.h"
#include "helper.h"
#include "postgres_query_generator.cpp"
#include "tao/pq/connection.hpp"
#include <cstdlib>
#include <cstring>
#include <queue>
#include <string>
#include <unistd.h>

extern pthread_t conn_threads[CONN_THREADS];
extern std::queue<connection_t *> conn_queue;
extern pthread_mutex_t conn_mtx;
extern pthread_cond_t conn_cond_var;
extern bool conn_done;

extern std::queue<char *> read_queue;
extern pthread_t read_threads[READ_THREADS];
extern pthread_mutex_t read_mtx;
extern pthread_cond_t read_cond_var;
extern bool read_done;

extern std::vector<std::tuple<std::string, time_t>> insert_statements;
extern pthread_t write_threads[WRITE_THREADS];
extern pthread_mutex_t write_mtx;
extern pthread_cond_t write_cond_var;
extern bool write_done;

PostgresQueryGenerator postgres_query_generator;
std::shared_ptr<tao::pq::connection> *db_postgres =
    (std::shared_ptr<tao::pq::connection> *)malloc(
        sizeof(std::shared_ptr<tao::pq::connection>) * WRITE_THREADS);

void *conn_worker_postgres(void *arg) {
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

void *read_worker_postgres(void *arg) {
  while (true) {
    pthread_mutex_lock(&read_mtx);
    while (read_queue.empty() && !read_done) {
      pthread_cond_wait(&read_cond_var, &read_mtx);
    }
    if (read_done && read_queue.empty()) {
      pthread_mutex_unlock(&read_mtx);
      break;
    }

    char *buf = read_queue.front();
    read_queue.pop();
    pthread_mutex_unlock(&read_mtx);

    Log entry = Serializer::serialize(buf);
    if (entry.is_vaild) {
      std::string query = postgres_query_generator.create_subquery(entry);
      pthread_mutex_lock(&write_mtx);
      insert_statements.push_back(std::make_tuple(query, time(NULL)));
      pthread_cond_broadcast(&write_cond_var);
      pthread_mutex_unlock(&write_mtx);
    }
  }

  free(arg);
  pthread_exit(nullptr);
}

void *write_worker_postgres(void *arg) {
  while (true) {
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
        postgres_query_generator.create_query(thread_copy);
    db_postgres[index]->execute(query_string);
    info("Save success.\n");
  }

  free(arg);
  pthread_exit(nullptr);
}

void run_postgres(Config config) {
  std::string db_connection_url =
      "postgres://" + config.user + ":" + config.password + "@" + config.host +
      ":" + std::to_string(config.port) + "/" + config.dbname;

  for (int i = 0; i < CONN_THREADS; ++i) {
    if (pthread_create(&conn_threads[i], NULL, *conn_worker_postgres, NULL) !=
        0) {
      error("create thread failed");
    }
  }

  for (int i = 0; i < READ_THREADS; ++i) {
    if (pthread_create(&read_threads[i], NULL, *read_worker_postgres, NULL) !=
        0) {
      error("create thread failed");
    }
  }

  for (int i = 0; i < WRITE_THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    // FIXME: try catch doesn't work here
    // I tried a bunch of stuff but nothing did
    // Just letting the app crash and print the error
    db_postgres[i] = tao::pq::connection::create(db_connection_url);
    if (pthread_create(&write_threads[i], NULL, *write_worker_postgres, a) !=
        0) {
      error("create thread failed");
    }
  }
}
