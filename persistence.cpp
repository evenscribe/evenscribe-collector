#include "clickhouse/client.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Persistence {

private:
public:
  static void save(clickhouse::Client *client, std::string table_name,
                   json j_object) {

    clickhouse::Block block;

    for (auto &[column_name, column_value] : j_object.items()) {
      auto column = std::make_shared<clickhouse::ColumnString>();
      column->Append(column_value);
      block.AppendColumn(column_name, column);
    }

    client->Insert(table_name, block);
  }
};
