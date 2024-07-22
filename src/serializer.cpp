#include "serializer.h"

Log Serializer::serialize(char *source) {
  Log log;
  cJSON *json = cJSON_ParseWithLength(source, BUFFER_SIZE);

  log.is_vaild = true;

  if (json == nullptr) {
    warn("evenscribe(log): invalid json received\n");
    warn((std::string(source) + "\n").c_str());
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  }

  cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(json, "Timestamp");
  if (!cJSON_IsNumber(timestamp)) {
    warn("evenscribe(log): Timestamp is either missing or not an integer\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  };
  log.Timestamp = timestamp->valuedouble;

  cJSON *trace_id = cJSON_GetObjectItemCaseSensitive(json, "TraceId");
  if (!cJSON_IsString(trace_id) || trace_id->valuestring == nullptr) {
    warn("evenscribe(log): TraceId is either missing or not a string\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  };
  log.TraceId = trace_id->valuestring;

  cJSON *span_id = cJSON_GetObjectItemCaseSensitive(json, "SpanId");
  if (!cJSON_IsString(span_id) || trace_id->valuestring == nullptr) {
    warn("evenscribe(log): SpanId is either missing or not a string\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  };
  log.TraceId = span_id->valuestring;

  cJSON *trace_flags = cJSON_GetObjectItemCaseSensitive(json, "TraceFlags");
  if (!cJSON_IsNumber(trace_flags)) {
    warn("evenscribe(log): TraceFlags is either missing or not an integer\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  };
  log.TraceFlags = trace_flags->valuedouble;

  cJSON *severity_text = cJSON_GetObjectItemCaseSensitive(json, "SeverityText");
  if (!cJSON_IsString(severity_text) || trace_id->valuestring == nullptr) {
    warn("evenscribe(log): SeverityText is either missing or not a string\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  };
  log.SeverityText = severity_text->valuestring;

  cJSON *severity_number =
      cJSON_GetObjectItemCaseSensitive(json, "SeverityNumber");
  if (!cJSON_IsNumber(severity_number)) {
    warn("evenscribe(log): SeverityNumber is either missing or not an "
         "integer\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  };
  log.SeverityNumber = severity_number->valuedouble;

  cJSON *service_name = cJSON_GetObjectItemCaseSensitive(json, "ServiceName");
  if (!cJSON_IsString(service_name) || trace_id->valuestring == nullptr) {
    warn("evenscribe(log): ServiceName is either missing or not a string\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  };
  log.ServiceName = service_name->valuestring;

  cJSON *body = cJSON_GetObjectItemCaseSensitive(json, "Body");
  if (!cJSON_IsString(body) || trace_id->valuestring == nullptr) {
    warn("evenscribe(log): Body is either missing or not a string\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  };
  log.Body = body->valuestring;

  cJSON *resource_attributes =
      cJSON_GetObjectItemCaseSensitive(json, "ResourceAttributes");
  if (!cJSON_IsObject(resource_attributes)) {
    warn("evenscribe(log): ResourceAttributes is either missing or not an "
         "object\n");
    log.is_vaild = false;
    cJSON_Delete(json);
    return log;
  }
  cJSON *attribute = nullptr;
  cJSON_ArrayForEach(attribute, resource_attributes) {
    if (!cJSON_IsString(attribute) || attribute->valuestring == nullptr) {
      warn("evenscribe(log): resource attribute key-value"
           " is or not a string\n");
      log.is_vaild = false;
      cJSON_Delete(json);
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
    cJSON_Delete(json);
    return log;
  }
  attribute = nullptr;
  cJSON_ArrayForEach(attribute, log_attributes) {
    if (!cJSON_IsString(attribute) || attribute->valuestring == nullptr) {
      warn("evenscribe(log): Log attribute key-value is not a string\n");
      log.is_vaild = false;
      cJSON_Delete(json);
      return log;
    }
    log.LogAttributes[attribute->string] = attribute->valuestring;
  }

  cJSON_Delete(json);

  return log;
}
