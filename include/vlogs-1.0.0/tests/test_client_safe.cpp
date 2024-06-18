#include <thread>
#include <vector>
#include <victorialogslib/client_safe.h>

void threadFunc(SafeClient &client, const std::string &payload) {
  client.insert(payload);
}

int main() {
  SafeClient client("http://localhost:9428");
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    auto payload =
        R"({ "log": { "level": "info", "message": "hello world" }, "date": "0", "stream": "stream1" })";

    threads.emplace_back(threadFunc, std::ref(client), payload);
  }

  for (auto &t : threads) {
    t.join();
  }
  return 0;
}
