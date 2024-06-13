#include "serializer.h"
#include "cJSON.h"
#include "log.h"

Log parse(std::string jsonString) {
  Log log;
  cJSON *json = cJSON_Parse(jsonString.c_str());

  log.is_vaild = true;

  if (json == nullptr) {
    warn("evenscribe(log): invalid json received\n");
    warn((jsonString + "\n").c_str());
    log.is_vaild = false;
    return log;
  }

  cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(json, "Timestamp");
  if (!cJSON_IsNumber(timestamp)) {
    warn("evenscribe(log): Timestamp is either missing or not a number\n");
    log.is_vaild = false;
    return log;
  }
  log.Timestamp = timestamp->valuedouble;

  cJSON *traceId = cJSON_GetObjectItemCaseSensitive(json, "TraceId");
  if (!cJSON_IsString(traceId) || traceId->valuestring == nullptr) {
    warn("evenscribe(log): TraceId is either missing or not a string\n");
    log.is_vaild = false;
    return log;
  }
  log.TraceId = traceId->valuestring;

  cJSON *spanId = cJSON_GetObjectItemCaseSensitive(json, "SpanId");
  if (!cJSON_IsString(spanId) || spanId->valuestring == nullptr) {
    warn("evenscribe(log): SpanId is either missing or not a string\n");
    log.is_vaild = false;
    return log;
  }
  log.SpanId = spanId->valuestring;

  cJSON *traceFlags = cJSON_GetObjectItemCaseSensitive(json, "TraceFlags");
  if (!cJSON_IsNumber(traceFlags)) {
    warn("evenscribe(log): TraceFlags is either missing or not a number\n");
    log.is_vaild = false;
    return log;
  }
  log.TraceFlags = traceFlags->valuedouble;

  cJSON *severityText = cJSON_GetObjectItemCaseSensitive(json, "SeverityText");
  if (!cJSON_IsString(severityText) || severityText->valuestring == nullptr) {
    warn("evenscribe(log): SeverityText is either missing or not a string\n");
    log.is_vaild = false;
    return log;
  }
  log.SeverityText = severityText->valuestring;

  cJSON *severityNumber =
      cJSON_GetObjectItemCaseSensitive(json, "SeverityNumber");
  if (!cJSON_IsNumber(severityNumber)) {
    warn("evenscribe(log): SeverityNumber is either missing or not a number\n");
    log.is_vaild = false;
    return log;
  }
  log.SeverityNumber = severityNumber->valuedouble;

  cJSON *serviceName = cJSON_GetObjectItemCaseSensitive(json, "ServiceName");
  if (!cJSON_IsString(serviceName) || serviceName->valuestring == nullptr) {
    warn("evenscribe(log): ServiceName is either missing or not a string\n");
    log.is_vaild = false;
    return log;
  }
  log.ServiceName = serviceName->valuestring;

  cJSON *body = cJSON_GetObjectItemCaseSensitive(json, "Body");
  if (!cJSON_IsString(body) || body->valuestring == nullptr) {
    warn("evenscribe(log): Body is either missing or not a string\n");
    log.is_vaild = false;
    return log;
  }
  log.Body = body->valuestring;

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
