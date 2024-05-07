#ifndef SOCKET
#define SOCKET

#include "param.h"
#include "persistence.cpp"
#include "serializer.cpp"
#include <clickhouse/client.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

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
    char buf[BUFFER_SIZE];
    memset(buf, 0x00, BUFFER_SIZE);

    while (true) {
      std::cout << "\nWaiting to accept a connection...\n";
      int client_socket = accept(server_socket, NULL, NULL);

      while (read(client_socket, buf, BUFFER_SIZE) > 0) {
        json j_object = Serializer::serialize(buf);
        persistence->save(TABLE_NAME, j_object);
        memset(buf, 0x00, BUFFER_SIZE);
        std::cout << "Save success.\n";
      }
    }
  }
};

#endif
