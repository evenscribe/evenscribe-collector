#ifndef SERIALIZER
#define SERIALIZER

#include <string>

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

class Serializer {
public:
  static LogEntry serialize(std::string source);
};

#endif // !SERIALIZER
