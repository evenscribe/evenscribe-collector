#ifndef QUERY_GENERATOR
#define QUERY_GENERATOR

#include "serializer.h"

class QueryGenerator {
public:
  static std::string query(const std::string table_name, const Log &entry);
};
#endif // !QUERY_GENERATOR
