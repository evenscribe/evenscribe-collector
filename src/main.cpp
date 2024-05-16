#include "socket.h"

int main() {
  // NOTE: check param.h to see the configs for the socket and clickhouse
  Socket socket = Socket();
  socket.handle_message();
  return 0;
}
