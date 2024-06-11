#include "serializer.h"
#include "cJSON.h"
#include <iostream>

Log parse(std::string jsonString) {
  Log log;
  cJSON *json = cJSON_Parse(jsonString.c_str());

  if (json == nullptr) {
    std::cerr << "Error parsing JSON" << std::endl;
    return log;
  }

  cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(json, "Timestamp");
  cJSON *traceId = cJSON_GetObjectItemCaseSensitive(json, "TraceId");
  cJSON *spanId = cJSON_GetObjectItemCaseSensitive(json, "SpanId");
  cJSON *traceFlags = cJSON_GetObjectItemCaseSensitive(json, "TraceFlags");
  cJSON *severityText = cJSON_GetObjectItemCaseSensitive(json, "SeverityText");
  cJSON *severityNumber =
      cJSON_GetObjectItemCaseSensitive(json, "SeverityNumber");
  cJSON *serviceName = cJSON_GetObjectItemCaseSensitive(json, "ServiceName");
  cJSON *body = cJSON_GetObjectItemCaseSensitive(json, "Body");

  if (cJSON_IsNumber(timestamp)) {
    log.Timestamp = timestamp->valuedouble;
  }
  if (cJSON_IsString(traceId) && traceId->valuestring != nullptr) {
    log.TraceId = traceId->valuestring;
  }
  if (cJSON_IsString(spanId) && spanId->valuestring != nullptr) {
    log.SpanId = spanId->valuestring;
  }
  if (cJSON_IsNumber(traceFlags)) {
    log.TraceFlags = traceFlags->valuedouble;
  }
  if (cJSON_IsString(severityText) && severityText->valuestring != nullptr) {
    log.SeverityText = severityText->valuestring;
  }
  if (cJSON_IsNumber(severityNumber)) {
    log.SeverityNumber = severityNumber->valuedouble;
  }
  if (cJSON_IsString(serviceName) && serviceName->valuestring != nullptr) {
    log.ServiceName = serviceName->valuestring;
  }
  if (cJSON_IsString(body) && body->valuestring != nullptr) {
    log.Body = body->valuestring;
  }

  cJSON *resourceAttributes =
      cJSON_GetObjectItemCaseSensitive(json, "ResourceAttributes");
  if (cJSON_IsObject(resourceAttributes)) {
    cJSON *attribute = nullptr;
    cJSON_ArrayForEach(attribute, resourceAttributes) {
      if (cJSON_IsString(attribute) && attribute->valuestring != nullptr) {
        log.ResourceAttributes[attribute->string] = attribute->valuestring;
      }
    }
  }

  cJSON *logAttributes =
      cJSON_GetObjectItemCaseSensitive(json, "LogAttributes");
  if (cJSON_IsObject(logAttributes)) {
    cJSON *attribute = nullptr;
    cJSON_ArrayForEach(attribute, logAttributes) {
      if (cJSON_IsString(attribute) && attribute->valuestring != nullptr) {
        log.LogAttributes[attribute->string] = attribute->valuestring;
      }
    }
  }

  cJSON_Delete(json);
  return log;
}

Log Serializer::serialize(std::string source) { return parse(source); }
