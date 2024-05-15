#include "socket.cpp"

int main() {
  Socket socket = Socket();
  clickhouse::Client client(clickhouse::ClientOptions()
                                .SetHost(CLICKHOUSE_HOST)
                                .SetPort(CLICKHOUSE_PORT));
  ClickhousePersistence persistence(&client);
  socket.handle_message(&persistence);
  return 0;
}
