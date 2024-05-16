#ifndef PERSISTENCE
#define PERSISTENCE

#include "clickhouse/client.h"
#include "serializer.cpp"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

class PersistenceManager {
public:
  virtual void save(const std::string table_name,
                    const LogEntry &entry) const = 0;
};

class ClickhousePersistence : public PersistenceManager {
private:
  clickhouse::Client *client;

  static std::string wrap(const std::string &str, const std::string &wrap) {
    return wrap + str + wrap;
  }

  static std::string between(const std::string &str, const std::string &start,
                             const std::string &end) {
    return start + str + end;
  }

  static std::string commaSeparate(const std::vector<std::string> &arr) {
    std::ostringstream oss;
    if (!arr.empty()) {
      oss << arr[0];
      for (size_t i = 1; i < arr.size(); ++i) {
        oss << "," << arr[i];
      }
    }
    return oss.str();
  }

public:
  explicit ClickhousePersistence(clickhouse::Client *client) : client(client) {}

  void save(const std::string table_name, const LogEntry &entry) const override {
    std::stringstream str;
    str << "\nINSERT INTO " << table_name << " VALUES "
        << between(
               commaSeparate({
                   std::to_string(entry.timestamp),
                   wrap(entry._msg, "'"),
                   between(commaSeparate({wrap(entry.log_owner.host_name, "'"),
                                          wrap(entry.log_owner.app_name, "'")}),
                           "(", ")"),
                   between(commaSeparate({wrap(entry.log.level, "'"),
                                          wrap(entry.log.message, "'")}),
                           "(", ")"),
               }),
               "(", ");");

    this->client->Execute(str.str());
  }
};
#endif // !PERSISTENCE
