#ifndef CLIENT_SAFE_H
#define CLIENT_SAFE_H

#include <mutex>
#include <string>

class SafeClient {
public:
  explicit SafeClient(const std::string &uri) noexcept;
  ~SafeClient() noexcept;
  class InsertResponse {
  public:
    InsertResponse(long statusCode, std::string text) noexcept
        : statusCode(statusCode), text(std::move(text)) {}
    long statusCode;
    std::string text;
  };
  InsertResponse insert(const std::string &payload);

private:
  const std::string base_uri;
  const std::string insert_uri;
  mutable std::mutex insert_mutex;
};

#endif // CLIENT_H
