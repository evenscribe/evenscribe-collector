#include "config.h"
#include "helper.h"
#include "postgres_query_generator.cpp"
#include "tao/pq/connection.hpp"
#include <queue>
#include <unistd.h>

extern pthread_t conn_threads[THREADS];
extern std::queue<connection_t *> conn_queue;
extern pthread_mutex_t conn_mtx;
extern pthread_cond_t conn_cond_var;
extern bool conn_done;

extern std::vector<std::tuple<std::string, time_t>> insert_statements;
extern pthread_t write_threads[THREADS];
extern pthread_mutex_t write_mtx;
extern pthread_cond_t write_cond_var;
extern bool write_done;

const uint16_t SAVE_THRESHOLD = 1000;
PostgresQueryGenerator postgres_query_generator;
std::shared_ptr<tao::pq::connection> *db =
    (std::shared_ptr<tao::pq::connection> *)malloc(
        sizeof(std::shared_ptr<tao::pq::connection>) * THREADS);

void *conn_worker(void *arg) {
  while (true) {
    int index = *((int *)arg);
    pthread_mutex_lock(&conn_mtx);
    while (conn_queue.empty() && !conn_done) {
      pthread_cond_wait(&conn_cond_var, &conn_mtx);
    }
    if (conn_done && conn_queue.empty()) {
      pthread_mutex_unlock(&conn_mtx);
      break;
    }

    connection_t *conn = conn_queue.front();
    conn->thread_id = index;
    conn_queue.pop();
    pthread_mutex_unlock(&conn_mtx);

    char buf[BUFFER_SIZE];
    while (is_socket_open(conn->client_socket) &&
           read(conn->client_socket, buf, BUFFER_SIZE) > 0) {
      Log entry = Serializer::serialize(buf);
      if (entry.is_vaild) {
        std::string query = postgres_query_generator.create_subquery(entry);
        pthread_mutex_lock(&write_mtx);
        insert_statements.push_back(std::make_tuple(query, time(NULL)));
        pthread_cond_broadcast(&write_cond_var);
        pthread_mutex_unlock(&write_mtx);
      }
    }

    delete conn;
  }

  free(arg);
  pthread_exit(nullptr);
}

void *write_worker(void *arg) {
  while (true) {
    // if (insert_statements.size() > 0) {
    //   bool should_insert =
    //       difftime(time(NULL), std::get<1>(insert_statements[0])) > 10;
    //   if (should_insert) {
    //     std::string query_string = get_query_from_bucket(insert_statements);
    //     try {
    //       db[0]->execute(query_string);
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
        postgres_query_generator.create_query(thread_copy);
    db[index]->execute(query_string);
    info("Save success.\n");
  }

  free(arg);
  pthread_exit(nullptr);
}

void run_postgres(Config config) {
  std::string db_connection_url =
      "postgres://" + config.user + ":" + config.password + "@" + config.host +
      ":" + std::to_string(config.port) + "/" + config.dbname;

  for (int i = 0; i < THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    // FIXME: try catch doesn't work here
    // I tried a bunch of stuff but nothing did
    // Just letting the app crash and print the error
    db[i] = tao::pq::connection::create(db_connection_url);
    if (pthread_create(&conn_threads[i], nullptr, *conn_worker, a) != 0) {
      error("create thread failed");
    }

    if (pthread_create(&write_threads[i], nullptr, *write_worker, a) != 0) {
      error("create thread failed");
    }
  }
}
