#include "config.h"

DatabaseKind get_database_kind(std::string database_kind_string) {
  static const std::unordered_map<std::string, DatabaseKind> database_kind_map =
      {
          {"postgres", POSTGRES},
          {"clickhouse", CLICKHOUSE},
      };

  auto item = database_kind_map.find(database_kind_string);
  if (item == database_kind_map.end()) {
    error("evenscribe(config): invalid string for databse_kind");
  }
  return item->second;
}

cJSON *get_json_field_value(cJSON *json, std::string field,
                            DataKind data_kind) {
  cJSON *field_value = cJSON_GetObjectItemCaseSensitive(json, field.c_str());
  switch (data_kind) {
  case STRING: {
    std::string error_msg =
        "evenscribe(config): " + field + " is missing or not a string\n";
    if (!cJSON_IsString(field_value) || field_value->valuestring == nullptr) {
      cJSON_Delete(json);
      error(error_msg.c_str());
    }
    break;
  }
  case INTEGER: {
    std::string error_msg =
        "evenscribe(config): " + field + " is missing or not a number\n";
    if (!cJSON_IsNumber(field_value)) {
      cJSON_Delete(json);
      error(error_msg.c_str());
    }
    break;
  }
  }

  return field_value;
}

Config deserializeJsonToConfig(const std::string &jsonString) {
  Config config;

  cJSON *json = cJSON_Parse(jsonString.c_str());
  if (json == nullptr) {
    error("evenscribe(config): error parsing config file\n");
  }

  config.database_kind = get_database_kind(
      get_json_field_value(json, "database_kind", STRING)->valuestring);
  config.host = get_json_field_value(json, "host", STRING)->valuestring;
  config.port = get_json_field_value(json, "port", INTEGER)->valuedouble;

  switch (config.database_kind) {
  case POSTGRES: {
    config.user = get_json_field_value(json, "user", STRING)->valuestring;
    config.password =
        get_json_field_value(json, "password", STRING)->valuestring;
    config.dbname = get_json_field_value(json, "dbname", STRING)->valuestring;
    break;
  }
  case CLICKHOUSE: {
    config.user = get_json_field_value(json, "user", STRING)->valuestring;
    config.password =
        get_json_field_value(json, "password", STRING)->valuestring;
    break;
  }
  }

  cJSON_Delete(json);

  return config;
}

char *get_config_file() {
  char *home = getenv("HOME");
  if (!home) {
    error("Unable to retrieve home directory.\n");
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

bool config_file_exists() {
  char *path = get_config_file();
  return file_exists(path);
}

char *read_file(const char *filePath) {
  FILE *file = fopen(filePath, "rb");
  if (file == NULL) {
    error("Could not open file\n");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(fileSize + 1);
  if (buffer == NULL) {
    error("Memory allocation failed\n");
    fclose(file);
    return NULL;
  }

  size_t bytesRead = fread(buffer, 1, fileSize, file);
  if (bytesRead != fileSize) {
    error("File read error\n");
    free(buffer);
    fclose(file);
    return NULL;
  }

  buffer[fileSize] = '\0';

  fclose(file);
  return buffer;
}

Config config_to_tuple() {
  if (!config_file_exists()) {
    error("evenscribe(config): config file doesn't exist : "
          "$HOME/.evenscriberc\n");
  }

  char *path = get_config_file();
  char *contents = read_file(path);

  return deserializeJsonToConfig(contents);
}