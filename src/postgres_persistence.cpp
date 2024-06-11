#include "persistence.h"
#include <tao/pq.hpp>

class PostgresPersistence : public Persistence {
private:
  std::shared_ptr<tao::pq::connection> client;

public:
  PostgresPersistence(std::shared_ptr<tao::pq::connection> client) {
    this->client = client;
  }

  void save(std::string query) const override { this->client->execute(query); }
};
