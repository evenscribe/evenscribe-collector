#ifndef SOCKET
#define SOCKET

#include "param.h"
#include "persistence.cpp"
#include "serializer.cpp"
#include <clickhouse/client.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

typedef struct {
  int client_socket;
  PersistenceManager *persistence;
} connection_t;

inline std::mutex mtx;
inline std::queue<connection_t *> task_queue;
inline pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;
inline pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
inline bool done = false;

inline void *process(void *arg) {
  connection_t *conn = (connection_t *)arg;

  if (!conn) {
    return nullptr;
  }

  char buf[BUFFER_SIZE];

  while (true) {
    ssize_t bytes_read = read(conn->client_socket, buf, BUFFER_SIZE);

    if (bytes_read <= 0) {
      break;
    }

    LogEntry entry = Serializer::serialize(buf);

    std::lock_guard<std::mutex> lock(mtx);
    conn->persistence->save(TABLE_NAME, entry);

    std::cout << "Save success.\n";
  }

  close(conn->client_socket);
  delete conn;
  return nullptr;
}

inline void *worker(void *arg) {
  while (true) {
    pthread_mutex_lock(&queue_mtx);
    while (task_queue.empty() && !done) {
      pthread_cond_wait(&cond_var, &queue_mtx);
    }

    if (done && task_queue.empty()) {
      pthread_mutex_unlock(&queue_mtx);
      break;
    }

    connection_t *conn = task_queue.front();
    task_queue.pop();
    pthread_mutex_unlock(&queue_mtx);

    process(conn);
  }
  return nullptr;
}

class Socket {
private:
  int server_socket;
  struct sockaddr_un addr;
  pthread_t thread_pool[8];

  void _sanitize() {
    remove(SOCKET_PATH);
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
  }

  void _bind() {
    if (bind(server_socket, (struct sockaddr *)&addr,
             sizeof(struct sockaddr_un)) == -1) {
      perror("Error: bind to socket failed");
      exit(1);
    }
  }

  void _listen() {
    if (listen(server_socket, MAXIMUM_CONNECTIONS) == -1) {
      perror("Error: listen on socket failed");
      exit(1);
    }
  }

public:
  Socket() {
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
      perror("Error: create socket failed");
      exit(1);
    }

    _sanitize();
    _bind();
    _listen();

    for (int i = 0; i < 8; ++i) {
      if (pthread_create(&thread_pool[i], nullptr, worker, nullptr) != 0) {
        perror("Error: create thread failed");
        exit(1);
      }
    }
  }

  ~Socket() {
    {
      pthread_mutex_lock(&queue_mtx);
      done = true;
      pthread_cond_broadcast(&cond_var);
      pthread_mutex_unlock(&queue_mtx);
    }

    for (int i = 0; i < THREADS; ++i) {
      pthread_join(thread_pool[i], nullptr);
    }

    close(server_socket);
    remove(SOCKET_PATH);
  }

  void handle_message(PersistenceManager *persistence) {
    while (true) {
      std::cout << "\nWaiting to accept a connection...\n";

      int client_socket = accept(server_socket, NULL, NULL);

      if (client_socket == -1) {
        perror("Error: accept connection failed");
        continue;
      }

      connection_t *conn = new connection_t{client_socket, persistence};

      pthread_mutex_lock(&queue_mtx);
      task_queue.push(conn);
      pthread_cond_signal(&cond_var);
      pthread_mutex_unlock(&queue_mtx);
    }
  }
};

#endif // !SOCKET
