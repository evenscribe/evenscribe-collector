#ifndef SOCKET
#define SOCKET

#include <sys/un.h>

class Socket {
private:
  int server_socket;
  struct sockaddr_un addr;

  void _sanitize();
  void _bind();
  void _listen();

public:
  Socket();
  ~Socket();
  void handle_message();
};

#endif // !SOCKET
