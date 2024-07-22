#ifndef CONFIG_H
#define CONFIG_H

#include "json.h"
#include "helper.h"
#include "log.h"
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

DatabaseKind get_database_kind(std::string database_kind_string);
cJSON *get_json_field_value(cJSON *json, std::string field, DataKind data_kind);
Config deserializeJsonToConfig(const std::string &jsonString);
char *get_config_file();
bool config_file_exists();
char *read_file(const char *filePath);
Config config_to_tuple();

#endif // CONFIG_H
