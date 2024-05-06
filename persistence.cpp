#include "clickhouse/client.h"

class Persistence {
public:
  static void save(clickhouse::Client *client, std::string table_name,
                   std::string column_name, std::string data) {
    clickhouse::Block block;
    auto insert_data = std::make_shared<clickhouse::ColumnString>();
    insert_data->Append(data);
    block.AppendColumn(column_name, insert_data);
    client->Insert(table_name, block);
  }
};
