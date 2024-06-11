#ifndef CONFIG
#define CONFIG

#include "cJSON.h"
#include "helper.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

enum DatabaseKind { POSTGRES, CLICKHOUSE };

struct Config {
  std::string host;
  int port;
  DatabaseKind database_kind;
};

static inline DatabaseKind get_database_kind(std::string database_kind_string) {
  static const std::unordered_map<std::string, DatabaseKind> database_kind_map =
      {
          {"postgres", POSTGRES},
          {"clickhouse", CLICKHOUSE},
      };

  auto item = database_kind_map.find(database_kind_string);
  if (item != database_kind_map.end()) {
    return item->second;
  } else {
    throw std::invalid_argument("Invalid string for DatabaseKind enum");
  }
}

static inline Config deserializeJsonToConfig(const std::string &jsonString) {
  Config config;

  // Parse the JSON string
  cJSON *json = cJSON_Parse(jsonString.c_str());
  if (json == nullptr) {
    throw std::runtime_error("Error parsing Config JSON");
  }

  // Extract clickhouse_host from JSON object
  cJSON *host =
      cJSON_GetObjectItemCaseSensitive(json, "host");
  if (!cJSON_IsString(host) ||
      host->valuestring == nullptr) {
    cJSON_Delete(json);
    throw std::runtime_error(
        "Error: clickhouse_host is missing or not a string");
  }
  config.host = host->valuestring;

  // Extract clickhouse_port from JSON object
  cJSON *port =
      cJSON_GetObjectItemCaseSensitive(json, "port");
  if (!cJSON_IsNumber(port)) {
    cJSON_Delete(json);
    throw std::runtime_error(
        "Error: clickhouse_port is missing or not a number");
  }
  config.port = port->valuedouble;

  cJSON *database_kind =
      cJSON_GetObjectItemCaseSensitive(json, "database_kind");
  if (!cJSON_IsString(database_kind) || database_kind->valuestring == nullptr) {
    cJSON_Delete(json);
    throw std::runtime_error(
        "Error: clickhouse_port is missing or not a number");
  }
  config.database_kind = get_database_kind(database_kind->valuestring);

  cJSON_Delete(json);

  return config;
}

static inline char *get_config_file() {
  char *home = getenv("HOME");
  if (!home) {
    error("evenscribe: unable to retrieve home directory! abort..\n");
  }
  const char *config_path = CONFIG_PATH;

  size_t size = strlen(home) + strlen(config_path) + 1;

  char *path = (char *)malloc(size);
  if (path == NULL) {
    error("Failed to allocate memory.\n");
  }

  strcpy(path, home);
  strcat(path, config_path);

  return path;
}

static inline bool config_file_exstis() {
  char *path = get_config_file();
  return file_exists(path);
}

static char *read_file(const char *filePath) {
  FILE *file = fopen(filePath, "rb"); // Open the file in binary mode
  if (file == NULL) {
    error("Could not open file");
    return NULL;
  }

  // Seek to the end of the file to determine its size
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET); // Reset to the beginning of the file

  // Allocate memory to hold the file contents
  char *buffer = (char *)malloc(fileSize + 1);
  if (buffer == NULL) {
    error("Memory allocation failed");
    fclose(file);
    return NULL;
  }

  // Read the file into the buffer
  size_t bytesRead = fread(buffer, 1, fileSize, file);
  if (bytesRead != fileSize) {
    error("File read error");
    free(buffer);
    fclose(file);
    return NULL;
  }

  // Null-terminate the buffer
  buffer[fileSize] = '\0';

  fclose(file);
  return buffer;
}

static Config config_to_tuple() {
  if (!config_file_exstis()) {
    error("Config file doesn't exist : $HOME/.evenscriberc");
    exit(1);
  }

  char *path = get_config_file();
  char *contents = read_file(path);
  try {
    return deserializeJsonToConfig(contents);
  } catch (...) {
    error("Error reading config file. Bad schema detected.");
    exit(1);
  }
}

#endif // ! CONFIG
