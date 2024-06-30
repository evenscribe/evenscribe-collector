#include "serializer.h"
#include "cJSON.h"
#include "log.h"
#include "param.h"

enum DataKind { INTEGER, STRING };

static inline cJSON *get_json_field_value(cJSON *json, std::string field,
                                          DataKind data_kind, Log *log) {
  cJSON *field_value = cJSON_GetObjectItemCaseSensitive(json, field.c_str());
  switch (data_kind) {
  case STRING: {
    std::string error_msg =
        "evenscribe(config): " + field + " is missing or not a string\n";
    if (!cJSON_IsString(field_value) || field_value->valuestring == nullptr) {
      cJSON_Delete(json);
      warn(error_msg.c_str());
      log->is_vaild = false;
    }
    break;
  }
  case INTEGER: {
    std::string error_msg =
        "evenscribe(config): " + field + " is missing or not a number\n";
    if (!cJSON_IsNumber(field_value)) {
      cJSON_Delete(json);
      warn(error_msg.c_str());
      log->is_vaild = false;
    }
    break;
  }
  }

  return field_value;
}

Log parse(std::string jsonString) {
  Log log;
  cJSON *json = cJSON_ParseWithLength(jsonString.c_str(), BUFFER_SIZE);

  log.is_vaild = true;

  if (json == nullptr) {
    warn("evenscribe(log): invalid json received\n");
    warn((jsonString + "\n").c_str());
    log.is_vaild = false;
    return log;
  }

  log.Timestamp =
      get_json_field_value(json, "Timestamp", INTEGER, &log)->valuedouble;
  log.TraceId =
      get_json_field_value(json, "TraceId", STRING, &log)->valuestring;
  log.SpanId = get_json_field_value(json, "SpanId", STRING, &log)->valuestring;
  log.TraceFlags =
      get_json_field_value(json, "TraceFlags", INTEGER, &log)->valuedouble;
  log.SeverityText =
      get_json_field_value(json, "SeverityText", STRING, &log)->valuestring;
  log.SeverityNumber =
      get_json_field_value(json, "SeverityNumber", INTEGER, &log)->valuedouble;
  log.ServiceName =
      get_json_field_value(json, "ServiceName", STRING, &log)->valuestring;
  log.Body = get_json_field_value(json, "Body", STRING, &log)->valuestring;

  cJSON *resourceAttributes =
      cJSON_GetObjectItemCaseSensitive(json, "ResourceAttributes");
  if (!cJSON_IsObject(resourceAttributes)) {
    warn("evenscribe(log): ResourceAttributes is either missing or not an "
         "object\n");
    log.is_vaild = false;
    return log;
  }
  cJSON *attribute = nullptr;
  cJSON_ArrayForEach(attribute, resourceAttributes) {
    if (!cJSON_IsString(attribute) || attribute->valuestring == nullptr) {
      warn("evenscribe(log): resource attribute key-value"
           " is or not a string\n");
      log.is_vaild = false;
      return log;
    }
    log.ResourceAttributes[attribute->string] = attribute->valuestring;
  }

  cJSON *logAttributes =
      cJSON_GetObjectItemCaseSensitive(json, "LogAttributes");
  if (!cJSON_IsObject(logAttributes)) {
    warn("evenscribe(log): LogAttributes is either missing or not an object\n");
    log.is_vaild = false;
    return log;
  }
  attribute = nullptr;
  cJSON_ArrayForEach(attribute, logAttributes) {
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

Log Serializer::serialize(std::string source) { return parse(source); }
