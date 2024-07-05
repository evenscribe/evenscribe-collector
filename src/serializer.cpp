#include "serializer.h"
#include "cJSON.h"
#include "log.h"
#include "param.h"
#include <string>

enum Field {
  TIMESTAMP,
  TRACE_ID,
  SPAN_ID,
  TRACE_FLAGS,
  SEVERITY_TEXT,
  SEVERITY_NUMBER,
  SERVICE_NAME,
  BODY,
};
enum DataKind { INTEGER, STRING };

static inline Field get_field(std::string database_kind_string) {
  static const std::unordered_map<std::string, Field> field_map = {
      {"Timestamp", TIMESTAMP},
      {"TraceId", TRACE_ID},
      {"SpanId", SPAN_ID},
      {"TraceFlags", TRACE_FLAGS},
      {"SeverityText", SEVERITY_TEXT},
      {"SeverityNumber", SEVERITY_NUMBER},
      {"ServiceName", SERVICE_NAME},
      {"Body", BODY},
  };

  auto item = field_map.find(database_kind_string);
  if (item == field_map.end()) {
    error("evenscribe(config): invalid string for databse_kind");
  }
  return item->second;
}

static void set_json_field_value(cJSON *json, std::string field,
                                 DataKind data_kind, Log *log) {
  cJSON *field_value = cJSON_GetObjectItemCaseSensitive(json, field.c_str());
  switch (data_kind) {
  case STRING: {
    std::string error_msg = "evenscribe(log): " + std::string(field) +
                            " is missing or not a string\n";
    if (!cJSON_IsString(field_value) || field_value->valuestring == nullptr) {
      warn(error_msg.c_str());
      log->is_vaild = false;
    } else {
      // clang-format off
      switch (get_field(field)) {
      case TRACE_ID: { log->TraceId = field_value->valuestring; break; }
      case SPAN_ID: { log->SpanId = field_value->valuestring; break; }
      case SEVERITY_TEXT: { log->SeverityText = field_value->valuestring; break; }
      case SERVICE_NAME: { log->ServiceName = field_value->valuestring; break; }
      case BODY: { log->Body = field_value->valuestring; break; }
      default: break;
      }
      // clang-format on
    }
    break;
  }
  case INTEGER: {
    std::string error_msg = "evenscribe(log): " + std::string(field) +
                            " is missing or not a number\n";
    if (!cJSON_IsNumber(field_value)) {
      warn(error_msg.c_str());
      log->is_vaild = false;
    } else {
      // clang-format off
      switch (get_field(field)) {
      case TIMESTAMP: { log->Timestamp = field_value->valuedouble; break; }
      case TRACE_FLAGS: { log->TraceFlags = field_value->valuedouble; break; }
      case SEVERITY_NUMBER: { log->SeverityNumber = field_value->valuedouble; break; }
      default: break;
      }
      // clang-format on
      break;
    }
  }
  }
}

Log parse(char *json_string)  {
  Log log;
  cJSON *json = cJSON_ParseWithLength(json_string, BUFFER_SIZE);

  log.is_vaild = true;

  if (json == nullptr) {
    warn("evenscribe(log): invalid json received\n");
    warn((std::string(json_string) + "\n").c_str());
    log.is_vaild = false;
    return log;
  }

  set_json_field_value(json, "Timestamp", INTEGER, &log);
  set_json_field_value(json, "TraceId", STRING, &log);
  set_json_field_value(json, "SpanId", STRING, &log);
  set_json_field_value(json, "TraceFlags", INTEGER, &log);
  set_json_field_value(json, "SeverityText", STRING, &log);
  set_json_field_value(json, "SeverityNumber", INTEGER, &log);
  set_json_field_value(json, "ServiceName", STRING, &log);
  set_json_field_value(json, "Body", STRING, &log);

  cJSON *resource_attributes =
      cJSON_GetObjectItemCaseSensitive(json, "ResourceAttributes");
  if (!cJSON_IsObject(resource_attributes)) {
    warn("evenscribe(log): ResourceAttributes is either missing or not an "
         "object\n");
    log.is_vaild = false;
    return log;
  }
  cJSON *attribute = nullptr;
  cJSON_ArrayForEach(attribute, resource_attributes) {
    if (!cJSON_IsString(attribute) || attribute->valuestring == nullptr) {
      warn("evenscribe(log): resource attribute key-value"
           " is or not a string\n");
      log.is_vaild = false;
      return log;
    }
    log.ResourceAttributes[attribute->string] = attribute->valuestring;
  }

  cJSON *log_attributes =
      cJSON_GetObjectItemCaseSensitive(json, "LogAttributes");
  if (!cJSON_IsObject(log_attributes)) {
    warn("evenscribe(log): LogAttributes is either missing or not an "
         "object\n");
    log.is_vaild = false;
    return log;
  }
  attribute = nullptr;
  cJSON_ArrayForEach(attribute, log_attributes) {
    if (!cJSON_IsString(attribute) || attribute->valuestring == nullptr) {
      warn("evenscribe(log): Log attribute key-value is not a string\n");
      log.is_vaild = false;
      return log;
    }
    log.LogAttributes[attribute->string] = attribute->valuestring;
  }

  cJSON_Delete(json);

  return log;
}

Log Serializer::serialize(char *source) { return parse(source); }
