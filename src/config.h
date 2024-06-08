#ifndef CONFIG
#define CONFIG

#include "cJSON.h"
#include "helper.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

struct Config {
  std::string clickhouse_host;
  int clickhouse_port;
};

static inline Config deserializeJsonToConfig(const std::string &jsonString) {
  Config config;

  // Parse the JSON string
  cJSON *json = cJSON_Parse(jsonString.c_str());
  if (json == nullptr) {
    throw std::runtime_error("Error parsing Config JSON");
  }

  // Extract clickhouse_host from JSON object
  cJSON *clickhouse_host =
      cJSON_GetObjectItemCaseSensitive(json, "clickhouse_host");
  if (!cJSON_IsString(clickhouse_host) ||
      clickhouse_host->valuestring == nullptr) {
    cJSON_Delete(json);
    throw std::runtime_error(
        "Error: clickhouse_host is missing or not a string");
  }
  config.clickhouse_host = clickhouse_host->valuestring;

  // Extract clickhouse_port from JSON object
  cJSON *clickhouse_port =
      cJSON_GetObjectItemCaseSensitive(json, "clickhouse_port");
  if (!cJSON_IsNumber(clickhouse_port)) {
    cJSON_Delete(json);
    throw std::runtime_error(
        "Error: clickhouse_port is missing or not a number");
  }
  config.clickhouse_port = clickhouse_port->valuedouble;

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
