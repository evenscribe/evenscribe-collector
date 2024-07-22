#ifndef CLICKHOUSE_QUERY_GENERATOR_H
#define CLICKHOUSE_QUERY_GENERATOR_H

#include "helper.h"
#include "serializer.h"
#include <deque>
#include <sstream>
#include <string>
#include <unordered_map>

namespace ClickhouseQueryGenerator {
template <typename K, typename V>
std::string unorderedMapToString(const std::unordered_map<K, V> &map);

std::string getValues(const Log &entry);

std::string create_subquery(Log &log);

std::string create_query(std::deque<std::string> bucket);
} // namespace ClickhouseQueryGenerator

#endif // CLICKHOUSE_QUERY_GENERATOR_H
