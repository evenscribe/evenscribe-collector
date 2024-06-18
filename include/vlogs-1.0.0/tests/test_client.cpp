#include <thread>
#include <vector>
#include <victorialogslib/client.h>

void threadFunc(const std::string &uri, const std::string &payload) {
  Client client(uri);
  client.insert(payload);
}

int main() {
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    auto payload =
        R"({ "log": { "level": "info", "message": "hello world" }, "date": "0", "stream": "stream1" })";
    threads.emplace_back(threadFunc, "http://localhost:9428", payload);
  }

  for (auto &t : threads) {
    t.join();
  }
  return 0;
}
