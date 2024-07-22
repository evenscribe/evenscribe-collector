#ifndef CLICKHOUSE_QUERY_GENERATOR_H
#define CLICKHOUSE_QUERY_GENERATOR_H

#include "helper.h"
#include "serializer.h"
#include <deque>
#include <sstream>
#include <string>
#include <unordered_map>

namespace ClickhouseQueryGenerator {
std::string create_subquery(Log &log);
std::string create_query(std::deque<std::string> bucket);
} // namespace ClickhouseQueryGenerator

#endif // CLICKHOUSE_QUERY_GENERATOR_H
