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
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

inline std::mutex mtx;

typedef struct {
  int client_socket;
  PersistenceManager *persistence;
} connection_t;

inline void *process(void *ptr) {
  if (!ptr) {
    pthread_exit(0);
  }

  connection_t *conn;
  conn = (connection_t *)ptr;

  char buf[BUFFER_SIZE];

  while (true) {
    ssize_t bytes_read = read(conn->client_socket, buf, BUFFER_SIZE);

    if (bytes_read <= 0) {
      break;
    }

    LogEntry entry = Serializer::serialize(buf);

    mtx.lock();
    conn->persistence->save(TABLE_NAME, entry);
    mtx.unlock();

    std::cout << "Save success.\n";
  }

  close(conn->client_socket);
  pthread_exit(0);
}

class Socket {
private:
  int server_socket;
  struct sockaddr_un addr;

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
  }

  ~Socket() {
    close(server_socket);
    remove(SOCKET_PATH);
  }

  void handle_message(PersistenceManager *persistence) {
    pthread_t thread;
    while (true) {
      std::cout << "\nWaiting to accept a connection...\n";

      int client_socket = accept(server_socket, NULL, NULL);
      if (client_socket == -1) {
        perror("Error: accept connection failed");
        continue;
      }

      connection_t *conn = new connection_t{client_socket, persistence};
      if (pthread_create(&thread, nullptr, process, conn) != 0) {
        perror("Error: create thread failed");
        close(client_socket);
        delete conn;
        continue;
      }

      pthread_detach(thread);
    }
  }
};

#endif // !SOCKET
