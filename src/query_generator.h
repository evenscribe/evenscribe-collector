#ifndef QUERY_GENERATOR
#define QUERY_GENERATOR

#include "serializer.h"

class QueryGenerator {
public:
  virtual std::string create_query(Log &log) const = 0;
};
#endif // !QUERY_GENERATOR
