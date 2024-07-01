#include "socket.h"
#include "log.h"
#include "param.h"
#include "run_clickhouse.cpp"
#include "run_postgres.cpp"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <queue>
#include <string.h>
#include <string>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

//
pthread_t conn_threads[CONN_THREADS];
std::queue<connection_t *> conn_queue;
pthread_mutex_t conn_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conn_cond_var = PTHREAD_COND_INITIALIZER;
bool conn_done = false;
//

//
std::vector<std::tuple<std::string, time_t>> insert_statements;
pthread_t write_threads[WRITE_THREADS];
pthread_mutex_t write_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t write_cond_var = PTHREAD_COND_INITIALIZER;
bool write_done = false;
//

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

Socket::Socket(Config config) {
  this->config = config;
  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_socket == -1) {
    error("create socket failed");
  }

  _sanitize();
  _bind();
  _listen();

  switch (config.database_kind) {
  case POSTGRES: {
    run_postgres(this->config);
    break;
  }
  case CLICKHOUSE: {
    run_clickhouse(this->config);
    break;
  }
  }
}

Socket::~Socket() {
  {
    pthread_mutex_lock(&conn_mtx);
    conn_done = true;
    pthread_cond_broadcast(&conn_cond_var);
    pthread_mutex_unlock(&conn_mtx);
  }

  for (int i = 0; i < CONN_THREADS; ++i) {
    pthread_join(conn_threads[i], nullptr);
  }

  for (int i = 0; i < WRITE_THREADS; ++i) {
    pthread_join(write_threads[i], nullptr);
  }

  close(server_socket);
  remove(SOCKET_PATH);
}

void Socket::handle_message() {
  while (true) {
    warn("Waiting for connection...\n");

    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket == -1) {
      warn("accept connection failed");
      continue;
    }

    connection_t *conn = new connection_t{client_socket};

    pthread_mutex_lock(&conn_mtx);
    conn_queue.push(conn);
    pthread_cond_broadcast(&conn_cond_var);
    pthread_mutex_unlock(&conn_mtx);
  }
}
