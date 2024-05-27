#include "serializer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void from_json(const json &j, Log &log) {
  j.at("Timestamp").get_to(log.Timestamp);
  j.at("TraceId").get_to(log.TraceId);
  j.at("SpanId").get_to(log.SpanId);
  j.at("TraceFlags").get_to(log.TraceFlags);
  j.at("SeverityText").get_to(log.SeverityText);
  j.at("SeverityNumber").get_to(log.SeverityNumber);
  j.at("ServiceName").get_to(log.ServiceName);
  j.at("Body").get_to(log.Body);
  j.at("ResourceAttributes").get_to(log.ResourceAttributes);
  j.at("LogAttributes").get_to(log.LogAttributes);
}

Log Serializer::serialize(std::string source) {
  json parsed = json::parse(source);
  return parsed;
}
