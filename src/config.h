#ifndef CONFIG_H
#define CONFIG_H

#include "helper.h"
#include "json.h"
#include "log.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>

enum DataKind { STRING, INTEGER };
enum DatabaseKind { POSTGRES, CLICKHOUSE };

struct Config {
  int port;
  std::string host;
  std::string user;
  std::string password;
  std::string dbname;
  DatabaseKind database_kind;
};

Config config_to_tuple();

#endif // CONFIG_H
