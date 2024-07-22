#ifndef POSTGRES_QUERY_GENERATOR_H
#define POSTGRES_QUERY_GENERATOR_H

#include "helper.h"
#include "serializer.h"
#include <deque>
#include <sstream>
#include <string>
#include <unordered_map>

namespace PostgresQueryGenerator {
std::string create_subquery(Log &log);
std::string create_query(std::deque<std::string> bucket);
} // namespace PostgresQueryGenerator

#endif // POSTGRES_QUERY_GENERATOR_H
