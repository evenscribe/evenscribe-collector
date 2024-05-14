#ifndef SOCKET
#define SOCKET

#include "param.h"
#include "persistence.cpp"
#include "serializer.cpp"
#include <clickhouse/client.h>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

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
  memset(buf, 0x00, BUFFER_SIZE);

  while (read(conn->client_socket, buf, BUFFER_SIZE) > 0) {
    LogEntry entry = Serializer::serialize(buf);
    conn->persistence->save(TABLE_NAME, entry);
    memset(buf, 0x00, BUFFER_SIZE);
    std::cout << "Save success.\n";
  }

  close(conn->client_socket);
  free(conn);
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
    strcpy(addr.sun_path, SOCKET_PATH);
  }

  void _bind() {
    if (bind(server_socket, (struct sockaddr *)&addr,
             sizeof(struct sockaddr_un)) == -1) {
      std::cout << "Error: bind to socket failed\n";
      exit(1);
    };
  }

  void _listen() { listen(server_socket, MAXIMUM_CONNECTIONS); }

public:
  Socket() {
    this->server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    _sanitize();
    _bind();
    _listen();
  }

  void handle_message(PersistenceManager *persistence) {
    pthread_t thread;
    connection_t *conn;

    while (true) {
      std::cout << "\nWaiting to accept a connection...\n";

      conn = (connection_t *)malloc(sizeof(connection_t));
      conn->client_socket = accept(server_socket, NULL, NULL);
      conn->persistence = persistence;

      if (conn->client_socket <= 0) {
        free(conn);
      } else {
        pthread_create(&thread, 0, process, (void *)conn);
        pthread_detach(thread);
      }
    }
  }
};

#endif // !SOCKET
