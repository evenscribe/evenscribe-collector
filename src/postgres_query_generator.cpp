#include "helper.h"
#include "query_generator.h"
#include <string>

class PostgresQueryGenerator : public QueryGenerator {
public:
  PostgresQueryGenerator() {}

  std::string create_query(Log &log) const override {
    std::stringstream str;
    str << between("INSERT INTO ", TABLE_NAME, " VALUES ")
        << between("(", getValues(log), ");");
    return str.str();
  }

  template <typename K, typename V>
  static inline std::string
  unorderedMapToString(const std::unordered_map<K, V> &map) {
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

  std::string getValues(const Log &entry) const {
    // clang-format off
    return commaSeparate({
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
      });
    }
};
