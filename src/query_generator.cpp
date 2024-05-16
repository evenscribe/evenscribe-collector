#include "query_generator.h"
#include "serializer.h"

#include <sstream>

std::string wrap(const std::string &str, const std::string &wrap) {
  return wrap + str + wrap;
}

std::string between(const std::string &start, const std::string &str,
                    const std::string &end) {
  return start + str + end;
}

std::string commaSeparate(const std::vector<std::string> &arr) {
  std::ostringstream oss;
  if (!arr.empty()) {
    oss << arr[0];
    for (size_t i = 1; i < arr.size(); ++i) {
      oss << "," << arr[i];
    }
  }
  return oss.str();
}

std::string getValues(const LogEntry &entry) {
  return commaSeparate({
      std::to_string(entry.timestamp),
      wrap(entry._msg, "'"),
      between("(",
              commaSeparate({wrap(entry.log_owner.host_name, "'"),
                             wrap(entry.log_owner.app_name, "'")}),
              ")"),
      between("(",
              commaSeparate(
                  {wrap(entry.log.level, "'"), wrap(entry.log.message, "'")}),
              ")"),
  });
}

std::string QueryGenerator::query(const std::string table_name,
                                  const LogEntry &entry) {
  std::stringstream str;
  str << between("INSERT INTO ", table_name, " VALUES ")
      << between("(", getValues(entry), ");");

  return str.str();
}
