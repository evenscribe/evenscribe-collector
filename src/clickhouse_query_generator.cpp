#include "helper.h"
#include "query_generator.h"

class ClickhouseQueryGenerator : public QueryGenerator {
public:
  ClickhouseQueryGenerator() {}

  std::string create_query(Log &log) const override {
    std::stringstream str;
    str << between("INSERT INTO ", TABLE_NAME, " VALUES ")
        << between("(", getValues(log), ");");
    return str.str();
  }
};
