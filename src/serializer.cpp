#ifndef SERIALIZER
#define SERIALIZER

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

struct LogOwner {
  std::string host_name;
  std::string app_name;
};

struct Log {
  std::string level;
  std::string message;
};

struct LogEntry {
  int timestamp;
  LogOwner log_owner;
  Log log;
  std::string _msg;
};

inline void from_json(const json &j, LogOwner &log_owner) {
  j.at("host_name").get_to(log_owner.host_name);
  j.at("app_name").get_to(log_owner.app_name);
}

inline void from_json(const json &j, Log &log) {
  j.at("level").get_to(log.level);
  j.at("message").get_to(log.message);
}

inline void from_json(const json &j, LogEntry &log_entry) {
  j.at("@timestamp").get_to(log_entry.timestamp);
  j.at("_msg").get_to(log_entry._msg);
  j.at("log").get_to(log_entry.log);
  j.at("log_owner").get_to(log_entry.log_owner);
}

class Serializer {

private:
  static std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
      return "";
    }
    return str.substr(start, end - start + 1);
  }

public:
  static LogEntry serialize(std::string source) {
    json parsed = json::parse(trim(source));
    return parsed;
  }
};

#endif // !SERIALIZER
