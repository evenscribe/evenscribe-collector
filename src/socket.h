#ifndef SOCKET
#define SOCKET

#include "config.h"
#include <sys/un.h>

class Socket {
private:
  int server_socket;
  struct sockaddr_un addr;
  Config config;

  void _sanitize();
  void _bind();
  void _listen();

public:
  Socket(Config config);
  ~Socket();
  void handle_message();
};

#endif // !SOCKET
