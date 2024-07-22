#include "query_generator_postgres.h"

namespace PostgresQueryGenerator {
std::string create_subquery(Log &log) {
  std::stringstream str;
  str << between("(", getValues(log), ")");
  return str.str();
}

template <typename K, typename V>
std::string unorderedMapToString(const std::unordered_map<K, V> &map) {
  std::ostringstream oss;
  oss << "'{";
  for (auto it = map.cbegin(); it != map.cend(); ++it) {
    if (it != map.cbegin()) {
      oss << ", ";
    }
    oss << wrap(it->first, "\"") << ":" << wrap(it->second, "\"");
  }
  oss << "}'";
  return oss.str();
}

std::string create_query(std::deque<std::string> bucket) {
  std::ostringstream query_string;
  query_string << between("INSERT INTO ", TABLE_NAME, " VALUES ")
               << commaSeparate(bucket) << ";";
  return query_string.str();
}

std::string getValues(const Log &entry) {
  std::deque<std::string> bucket = {
      std::to_string(entry.Timestamp),
      wrap(entry.TraceId, "'"),
      wrap(entry.SpanId, "'"),
      std::to_string(entry.TraceFlags),
      wrap(entry.SeverityText, "'"),
      std::to_string(entry.SeverityNumber),
      wrap(entry.ServiceName, "'"),
      wrap(entry.Body, "'"),
      unorderedMapToString(entry.ResourceAttributes),
      unorderedMapToString(entry.LogAttributes),
  };

  return commaSeparate(bucket);
}
} // namespace PostgresQueryGenerator
