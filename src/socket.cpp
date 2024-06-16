#include "socket.h"
#include "clickhouse_persistence.cpp"
#include "clickhouse_query_generator.cpp"
#include "log.h"
#include "param.h"
#include "postgres_persistence.cpp"
#include "postgres_query_generator.cpp"
#include "serializer.h"
#include <clickhouse/client.h>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <queue>
#include <sys/socket.h>
#include <unistd.h>

//
ClickhouseQueryGenerator
    clickhouse_query_generator; // should implement create_query
                                // function from "query_generator.h"
clickhouse::Client *clickhouse_db = (clickhouse::Client *)malloc(
    sizeof(clickhouse::Client) * THREADS); // clickhouse instance array
ClickhousePersistence *clickhouse_db_connections =
    (ClickhousePersistence *)malloc(
        sizeof(ClickhousePersistence) *
        THREADS); // should implement save function from "persistence.h"
//

//
PostgresQueryGenerator postgres_query_generator;
PostgresPersistence *postgres_db_connections =
    (PostgresPersistence *)malloc(sizeof(PostgresPersistence) * THREADS);
//

typedef struct {
  int client_socket;
  int thread_id;
} connection_t;

std::mutex mtx;
pthread_t thread_pool[THREADS];
std::queue<connection_t *> task_queue;

pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
bool done = false;

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
        clickhouse_db_connections[conn->thread_id].save(query);
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
  } else {
    warn("Socket buffer read error.");
  }

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

void process_postgres(void *arg) {
  connection_t *conn = (connection_t *)arg;

  if (!conn) {
    return;
  }

  char buf[BUFFER_SIZE];

  if (read(conn->client_socket, buf, BUFFER_SIZE) > 0) {
    Log entry = Serializer::serialize(buf);
    if (entry.is_vaild) {
      std::string query = postgres_query_generator.create_query(entry);
      const char *response;
      try {
        postgres_db_connections[conn->thread_id].save(query);
        response = "OK";
      } catch (const std::exception &e) {
        warn(e.what());
        response = "NO";
      }
      if (write(conn->client_socket, response, sizeof(char) * 2) == -1) {
        warn("Write error.");
      }
      info("Save success.\n");
    }
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

void initialize_postgres() {
  free(clickhouse_db);
  free(clickhouse_db_connections);

  // FIXME: try catch doesn't work here
  // I tried a bunch of stuff but nothing did
  // Just letting the app crash and print the error
  for (int i = 0; i < THREADS; ++i) {
    new (&postgres_db_connections[i])
        PostgresPersistence(tao::pq::connection::create(
            "postgres://"
            "postgres.oznsvtaespxsxlczrgaf:AVNS_xiRKk2llMMZ4ZsdYWKT@aws-0-us-"
            "east-1.pooler.supabase.com:6543/postgres"));
  }
  int i;
  for (i = 0; i < THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    if (pthread_create(&thread_pool[i], nullptr, *worker_postgres, a) != 0) {
      error("create thread failed");
    }
  }
}

void initialize_clickhouse(Config config) {
  free(postgres_db_connections);
  for (int i = 0; i < THREADS; ++i) {
    // FIXME: this shit right here throws error
    // but cannot be caught for some reason
    // doesn't even print a debug message
    new (&clickhouse_db[i]) clickhouse::Client(
        clickhouse::ClientOptions().SetHost(config.host).SetPort(config.port));
    new (&clickhouse_db_connections[i])
        ClickhousePersistence(&clickhouse_db[i]);
  }
  int i;
  for (i = 0; i < THREADS; ++i) {
    int *a = (int *)malloc(sizeof(int));
    *a = i;
    if (pthread_create(&thread_pool[i], nullptr, *worker_clickhouse, a) != 0) {
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
    initialize_postgres();
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
    pthread_join(thread_pool[i], nullptr);
  }

  switch (this->config.database_kind) {
  case POSTGRES: {
    free(postgres_db_connections);
    break;
  }
  case CLICKHOUSE: {
    free(clickhouse_db);
    free(clickhouse_db_connections);
    break;
  }
  }

  close(server_socket);
  remove(SOCKET_PATH);
}

void Socket::handle_message() {
  while (true) {
    info("\nWaiting to accept a connection...\n");

    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket == -1) {
      warn("accept connection failed");
      continue;
    }

    connection_t *conn = new connection_t{client_socket};

    pthread_mutex_lock(&queue_mtx);
    task_queue.push(conn);
    pthread_cond_broadcast(&cond_var);
    pthread_mutex_unlock(&queue_mtx);
  }
}
