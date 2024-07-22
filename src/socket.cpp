#include "socket.h"

std::deque<connection_t *> conn_queue;
pthread_t conn_threads[CONN_THREADS];
pthread_mutex_t conn_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conn_cond_var = PTHREAD_COND_INITIALIZER;

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
    Postgres::run(this->config);
    break;
  }
  case CLICKHOUSE: {
    Clickhouse::run(this->config);
    break;
  }
  }
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
    conn_queue.push_front(conn);
    pthread_cond_broadcast(&conn_cond_var);
    pthread_mutex_unlock(&conn_mtx);
  }
}
