#include "helper.h"
#include "serializer.h"
#include <string>

class ClickhouseQueryGenerator {
public:
  ClickhouseQueryGenerator() {}

  std::string create_subquery(Log &log) {
    std::stringstream str;
    str << between("(", getValues(log), ")");
    return str.str();
  }

  template <typename K, typename V>
  static inline std::string
  unorderedMapToString(const std::unordered_map<K, V> &map) {
    std::ostringstream oss;
    oss << "{";
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
      if (it != map.cbegin()) {
        oss << ", ";
      }
      oss << wrap(it->first, "'") << ":" << wrap(it->second, "'");
    }
    oss << "}";
    return oss.str();
  }

  static std::string
  create_query(std::vector<std::tuple<std::string, time_t>> bucket) {
    std::vector<std::string> query_strings;
    query_strings.reserve(bucket.size());
    for (const auto &t : bucket) {
      query_strings.push_back(std::get<0>(t));
    }
    std::ostringstream query_string;
    query_string << between("INSERT INTO ", TABLE_NAME, " VALUES ")
                 << commaSeparate(query_strings) << ";";
    return query_string.str();
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
