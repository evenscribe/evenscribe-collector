#ifndef CLIENT_H
#define CLIENT_H

#include <string>

class Client {
public:
  explicit Client(const std::string &uri) noexcept;
  ~Client() noexcept;
  class InsertResponse {
  public:
    InsertResponse(long statusCode, std::string text) noexcept
        : statusCode(statusCode), text(std::move(text)) {}
    long statusCode;
    std::string text;
  };
  InsertResponse insert(const std::string &payload) const;

private:
  const std::string base_uri;
  const std::string insert_uri;
};

#endif // CLIENT_H
