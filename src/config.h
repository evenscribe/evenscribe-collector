#ifndef CONFIG
#define CONFIG

#include "helper.h"
#include "log.h"
#include <nlohmann/json.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using json = nlohmann::json;

struct Config {
  std::string clickhouse_host;
  int clickhouse_port;
};

static void from_json(const json &j, Config &config) {
  j.at("clickhouse_host").get_to(config.clickhouse_host);
  j.at("clickhouse_port").get_to(config.clickhouse_port);
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
    perror("Could not open file");
    return NULL;
  }

  // Seek to the end of the file to determine its size
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET); // Reset to the beginning of the file

  // Allocate memory to hold the file contents
  char *buffer = (char *)malloc(fileSize + 1);
  if (buffer == NULL) {
    perror("Memory allocation failed");
    fclose(file);
    return NULL;
  }

  // Read the file into the buffer
  size_t bytesRead = fread(buffer, 1, fileSize, file);
  if (bytesRead != fileSize) {
    perror("File read error");
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
    return json::parse(contents);
  } catch (...) {
    error("Error reading config file. Bad schema detected.");
    exit(1);
  }
}

#endif // ! CONFIG
