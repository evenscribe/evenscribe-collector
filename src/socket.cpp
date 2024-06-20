#include "socket.h"
#include "clickhouse_query_generator.cpp"
#include "log.h"
#include "param.h"
#include "postgres_query_generator.cpp"
#include "serializer.h"
#include "tao/pq/connection.hpp"
#include <clickhouse/client.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <queue>
#include <string.h>
#include <string>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <victorialogslib/client.h>

//
ClickhouseQueryGenerator clickhouse_query_generator;
clickhouse::Client *clickhouse_db =
    (clickhouse::Client *)malloc(sizeof(clickhouse::Client) * THREADS);
//

//
PostgresQueryGenerator postgres_query_generator;
std::shared_ptr<tao::pq::connection> *postgres_db =
    (std::shared_ptr<tao::pq::connection> *)malloc(
        sizeof(std::shared_ptr<tao::pq::connection>) * THREADS);
//

using VictoriaClient = Client;
VictoriaClient *victoria_db =
    (VictoriaClient *)malloc(sizeof(VictoriaClient) * THREADS);
//

typedef struct {
  int client_socket;
  int thread_id;
} connection_t;

pthread_t thread_pool_conn[THREADS];
std::queue<connection_t *> task_queue;

pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
bool done = false;

//
std::vector<std::tuple<std::string, time_t>> insert_statements;
pthread_t thread_pool_inserts[THREADS];

pthread_mutex_t inserts_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t inserts_cond_var = PTHREAD_COND_INITIALIZER;
bool done_inserts = false;
//

void process_clickhouse(void *arg) {
  connection_t *conn = (connection_t *)arg;

  if (!conn) {
    return;
  }

  char buf[BUFFER_SIZE];

  if (read(conn->client_socket, buf, BUFFER_SIZE) > 0) {
    Log entry = Serializer::serialize(buf);
    if (entry.is_vaild) {
      std::string query = clickhouse_query_generator.create_query(entry);
      const char *response;
      try {
        clickhouse_db[conn->thread_id].Execute(query);
        response = "OK";
      } catch (...) {
        warn("");
        response = "NO";
      }
      if (write(conn->client_socket, response, sizeof(char) * 2) == -1) {
        warn("Write error.");
      }
      std::cout << "Save success.\n";
    }
  }
  // else {
  //   warn("Socket buffer read error.");
  // }

  close(conn->client_socket);
  delete conn;
}

void *worker_clickhouse(void *arg) {
  while (true) {
    int index = *((int *)arg);
    pthread_mutex_lock(&queue_mtx);

    while (task_queue.empty() && !done) {
      pthread_cond_wait(&cond_var, &queue_mtx);
    }

    if (done && task_queue.empty()) {
      pthread_mutex_unlock(&queue_mtx);
      break;
    }

    connection_t *conn = task_queue.front();
    conn->thread_id = index;
    task_queue.pop();
    pthread_mutex_unlock(&queue_mtx);
    process_clickhouse(conn);
  }

  free(arg);
  pthread_exit(nullptr);
}

std::string read_socket(int socket_fd) {
  std::string sockResponseString;
  auto can_read = [](int s) -> bool {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(s, &read_set);
    struct timeval timeout {};
    int rc = select(s + 1, &read_set, NULL, NULL, &timeout);
    return (rc == 1) && FD_ISSET(s, &read_set);
  };
  auto do_read = [&sockResponseString](int s) -> bool {
    char buf[BUFFER_SIZE];
    int nbytes = recv(s, buf, sizeof(buf), 0);
    if (nbytes <= 0)
      return false;
    sockResponseString += std::string(buf, static_cast<size_t>(nbytes));
    return true;
  };
  bool done{};
  while (!done) {
    if (can_read(socket_fd) && do_read(socket_fd)) {
      while (!done) {
        if (!can_read(socket_fd))
          done = true;
        do_read(socket_fd);
      }
    }
  }
  return sockResponseString;
}

bool is_socket_open(int sockfd) {
  char buffer;
  ssize_t result = recv(sockfd, &buffer, 1, MSG_PEEK);

  if (result == 0) {
    return false;
  } else if (result < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return true;
    } else {
      return false;
    }
  }

  return true;
}

void process_postgres(void *arg) {
  connection_t *conn = (connection_t *)arg;

  if (!conn) {
    return;
  }

  char buf[BUFFER_SIZE];

  while (is_socket_open(conn->client_socket) &&
         read(conn->client_socket, buf, BUFFER_SIZE) > 0) {

    Log entry = Serializer::serialize(buf);
    if (entry.is_vaild) {
      std::string query = postgres_query_generator.create_query(entry);
      pthread_mutex_lock(&inserts_mtx);
      insert_statements.push_back(std::make_tuple(query, time(NULL)));
      pthread_cond_broadcast(&inserts_cond_var);
      pthread_mutex_unlock(&inserts_mtx);
    }
  }

  delete conn;
}

void process_victoria(void *arg) {
  connection_t *conn = (connection_t *)arg;

  if (!conn) {
    return;
  }

  char buf[BUFFER_SIZE];

  if (read(conn->client_socket, buf, BUFFER_SIZE) > 0) {
    const char *response;
    try {
      victoria_db[conn->thread_id].insert(buf);
      response = "OK";
    } catch (...) {
      warn("error is happeing hererersatoehu ");
      response = "NO";
    }
    if (write(conn->client_socket, response, sizeof(char) * 2) == -1) {
      warn("Write error.");
    }
    info("Save success.\n");
  } else {
    warn("Socket buffer read error.");
  }

  close(conn->client_socket);
  delete conn;
}

void *worker_postgres(void *arg) {
  while (true) {
    int index = *((int *)arg);
    pthread_mutex_lock(&queue_mtx);

    while (task_queue.empty() && !done) {
      pthread_cond_wait(&cond_var, &queue_mtx);
    }

    if (done && task_queue.empty()) {
      pthread_mutex_unlock(&queue_mtx);
      break;
    }

    connection_t *conn = task_queue.front();
    conn->thread_id = index;
    task_queue.pop();
    pthread_mutex_unlock(&queue_mtx);
    process_postgres(conn);
  }

  free(arg);
  pthread_exit(nullptr);
}

const uint16_t SAVE_THRESHOLD = 1000;

void *worker_postgres_db(void *arg) {
  while (true) {
    int index = *((int *)arg);
    pthread_mutex_lock(&inserts_mtx);

    while (insert_statements.size() < SAVE_THRESHOLD && !done_inserts) {
      pthread_cond_wait(&inserts_cond_var, &inserts_mtx);
    }

    if (done_inserts && insert_statements.size() < SAVE_THRESHOLD) {
      pthread_mutex_unlock(&inserts_mtx);
      break;
    }

    std::vector<std::tuple<std::string, time_t>> thread_copy(
        insert_statements.begin(), insert_statements.begin() + SAVE_THRESHOLD);
    insert_statements.erase(insert_statements.begin(),
                            insert_statements.begin() + SAVE_THRESHOLD);
    pthread_mutex_unlock(&inserts_mtx);

    std::string query_string = get_query_from_bucket(thread_copy);
    try {
      postgres_db[index]->execute(query_string);
    } catch (const std::exception &e) {
      warn(e.what());
    }
    info("Save success.\n");
  }

  free(arg);
  pthread_exit(nullptr);
}

void *worker_victoria(void *arg) {
  while (true) {
    int index = *((int *)arg);
    pthread_mutex_lock(&queue_mtx);

    while (task_queue.empty() && !done) {
      pthread_cond_wait(&cond_var, &queue_mtx);
    }

    if (done && task_queue.empty()) {
      pthread_mutex_unlock(&queue_mtx);
      break;
    }

    connection_t *conn = task_queue.front();
    conn->thread_id = index;
    task_queue.pop();
    pthread_mutex_unlock(&queue_mtx);
    process_victoria(conn);
  }

  free(arg);
  pthread_exit(nullptr);
}

void Socket::_sanitize() {
  remove(SOCKET_PATH);
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
}

void Socket::_bind() {
  if (bind(server_socket, (struct sockaddr *)&addr,
           sizeof(struct sockaddr_un)) == -1) {
    error("Error: bind to socket failed");
  }
}

void Socket::_listen() {
  if (listen(server_socket, MAXIMUM_CONNECTIONS) == -1) {
    error("Error: listen on socket failed");
  }
}

void initialize_postgres(Config config) {
  free(clickhouse_db);

  std::string db_connection_url =
      "postgres://" + config.user + ":" + config.password + "@" + config.host +
      ":" + std::to_string(config.port) + "/" + config.dbname;

  // FIXME: try catch doesn't work here
  // I tried a bunch of stuff but nothing did
  // Just letting the app crash and print the error
  for (int i = 0; i < THREADS; ++i) {
    postgres_db[i] = tao::pq::connection::create(db_connection_url);
  }
  int i;
  for (i = 0; i < THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    if (pthread_create(&thread_pool_conn[i], nullptr, *worker_postgres, a) !=
        0) {
      error("create thread failed");
    }
  }

  int j;
  for (j = 0; j < THREADS; ++j) {
    int *a = (int *)malloc(sizeof(int));
    *a = j;
    if (pthread_create(&thread_pool_conn[j], nullptr, *worker_postgres_db, a) !=
        0) {
      error("create thread failed");
    }
  }
}

void initialize_clickhouse(Config config) {
  for (int i = 0; i < THREADS; ++i) {
    // FIXME: this shit right here throws error
    // but cannot be caught for some reason
    // doesn't even print a debug message
    new (&clickhouse_db[i]) clickhouse::Client(
        clickhouse::ClientOptions().SetHost(config.host).SetPort(config.port));
  }
  int i;
  for (i = 0; i < THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    if (pthread_create(&thread_pool_conn[i], nullptr, *worker_clickhouse, a) !=
        0) {
      error("create thread failed");
    }
  }
}

void initialize_victoria(Config config) {
  for (int i = 0; i < THREADS; ++i) {
    new (&victoria_db[i]) VictoriaClient("http://localhost:9428");
  }
  int i;
  for (i = 0; i < THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    if (pthread_create(&thread_pool_conn[i], nullptr, *worker_victoria, a) !=
        0) {
      error("create thread failed");
    }
  }
}

// void fd_set_nb(int fd) {
//   errno = 0;
//   int flags = fcntl(fd, F_GETFL, 0);
//   if (errno) {
//     error("couldn't set non-blocking");
//   }
//   flags |= O_NONBLOCK;
//   errno = 0;
//   (void)fcntl(fd, F_SETFL, flags);
//   if (errno) {
//     error("couldn't set non-blocking");
//   }
// }

Socket::Socket(Config config) {
  this->config = config;
  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_socket == -1) {
    error("create socket failed");
  }

  _sanitize();
  _bind();
  _listen();

  // fd_set_nb(server_socket);

  switch (config.database_kind) {
  case POSTGRES: {
    initialize_postgres(this->config);
    break;
  }
  case CLICKHOUSE: {
    initialize_clickhouse(this->config);
    break;
  }
  }
}

Socket::~Socket() {
  {
    pthread_mutex_lock(&queue_mtx);
    done = true;
    pthread_cond_broadcast(&cond_var);
    pthread_mutex_unlock(&queue_mtx);
  }

  for (int i = 0; i < THREADS; ++i) {
    pthread_join(thread_pool_conn[i], nullptr);
  }

  switch (this->config.database_kind) {
  case POSTGRES: {
    break;
  }
  case CLICKHOUSE: {
    free(clickhouse_db);
    break;
  }
  }

  close(server_socket);
  remove(SOCKET_PATH);
}

void Socket::handle_message() {
  while (true) {
    warn("Waiting for connection...\n");
    // for (auto &bucket : insert_statements) {
    //   if (bucket.size() > 0) {
    //     bool should_insert = difftime(time(NULL), std::get<1>(bucket[0])) >
    //     10; if (should_insert) {
    //       std::string query_string = get_query_from_bucket(bucket);
    //       try {
    //         postgres_db[0]->execute(query_string);
    //       } catch (const std::exception &e) {
    //         warn(e.what());
    //       }
    //       info("Save success auto.\n");
    //       bucket.clear();
    //     }
    //   }
    // }

    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket == -1) {
      warn("accept connection failed");
      // continue;
    }

    connection_t *conn = new connection_t{client_socket};

    pthread_mutex_lock(&queue_mtx);
    task_queue.push(conn);
    pthread_cond_broadcast(&cond_var);
    pthread_mutex_unlock(&queue_mtx);
  }
}
