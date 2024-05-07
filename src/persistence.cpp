#include "clickhouse/client.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class PersistenceManager {
public:
  virtual void save(std::string table_name, json j_object) const = 0;
};

class ClickhousePersistence : public PersistenceManager {
private:
  clickhouse::Client *client;

public:
  ClickhousePersistence(clickhouse::Client *client) { this->client = client; }

  void save(std::string table_name, json j_object) const override {
    clickhouse::Block block;

    for (auto &[column_name, column_value] : j_object.items()) {
      auto column = std::make_shared<clickhouse::ColumnString>();
      column->Append(column_value);
      block.AppendColumn(column_name, column);
    }

    this->client->Insert(table_name, block);
  }
};
