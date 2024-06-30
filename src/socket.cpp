#include "socket.h"
#include "clickhouse_query_generator.cpp"
#include "log.h"
#include "param.h"
#include "run_postgres.cpp"
#include "serializer.h"
#include <clickhouse/client.h>
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
ClickhouseQueryGenerator clickhouse_query_generator;
clickhouse::Client *clickhouse_db =
    (clickhouse::Client *)malloc(sizeof(clickhouse::Client) * WRITE_THREADS);
//

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

void process_clickhouse(void *arg) {
  connection_t *conn = (connection_t *)arg;

  if (!conn) {
    return;
  }

  char buf[BUFFER_SIZE];

  if (is_socket_open(conn->client_socket) &&
      read(conn->client_socket, buf, BUFFER_SIZE) > 0) {
    Log entry = Serializer::serialize(buf);
    if (entry.is_vaild) {
      std::string query = clickhouse_query_generator.create_query(entry);
      clickhouse_db[conn->thread_id].Execute((query));
      // if (write(conn->client_socket, response, sizeof(char) * 2) == -1) {
      //   warn("Write error.");
      // }
      warn("Save success.\n");
    }
  }

  close(conn->client_socket);
  delete conn;
}

void *worker_clickhouse(void *arg) {
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
    process_clickhouse(conn);
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

void initialize_clickhouse(Config config) {
  for (int i = 0; i < WRITE_THREADS; ++i) {
    // FIXME: this shit right here throws error
    // but cannot be caught for some reason
    // doesn't even print a debug message
    new (&clickhouse_db[i]) clickhouse::Client(
        clickhouse::ClientOptions()
            .SetHost("tgn2urxfn9.us-east-2.aws.clickhouse.cloud")
            .SetPort(9440)
            .SetUser("default")
            .SetPassword("gZ_m_0~ZCKgKT")
            .SetSSLOptions(clickhouse::ClientOptions::SSLOptions()));
  }
  for (int i = 0; i < CONN_THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    if (pthread_create(&conn_threads[i], nullptr, *worker_clickhouse, a) != 0) {
      error("create thread failed");
    }
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
    initialize_clickhouse(this->config);
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
