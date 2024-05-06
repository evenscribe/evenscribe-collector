#include "socket.cpp"

int main() {
  Socket socket = Socket("/tmp/olympus.sock");
  clickhouse::Client client(
      clickhouse::ClientOptions().SetHost(CLICKHOUSE_HOST));
  socket.handle_message(&client);
}
