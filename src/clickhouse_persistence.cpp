#include "clickhouse/client.h"
#include "persistence.h"

class ClickhousePersistence : public Persistence {
private:
  clickhouse::Client *client;

public:
  ClickhousePersistence(clickhouse::Client *client) { this->client = client; }

  void save(std::string query) const override { this->client->Execute(query); }
};
