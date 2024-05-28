#include "query_generator.h"
#include "serializer.h"

#include <sstream>
#include <string>
#include <vector>

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

template <typename K, typename V>
std::string unorderedMapToString(const std::unordered_map<K, V> &map) {
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

std::string getValues(const Log &entry) {
  // clang-format off
  return commaSeparate({
          between("toDateTime64(",wrap(entry.Timestamp, "'"),  ",9)"),
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

std::string QueryGenerator::query(const std::string table_name,
                                  const Log &entry) {
  std::stringstream str;
  str << between("INSERT INTO ", table_name, " VALUES ")
      << between("(", getValues(entry), ");");

  return str.str();
}
