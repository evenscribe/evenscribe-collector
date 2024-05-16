#include "serializer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void from_json(const json &j, LogOwner &log_owner) {
  j.at("host_name").get_to(log_owner.host_name);
  j.at("app_name").get_to(log_owner.app_name);
}

void from_json(const json &j, Log &log) {
  j.at("level").get_to(log.level);
  j.at("message").get_to(log.message);
}

void from_json(const json &j, LogEntry &log_entry) {
  j.at("@timestamp").get_to(log_entry.timestamp);
  j.at("_msg").get_to(log_entry._msg);
  j.at("log").get_to(log_entry.log);
  j.at("log_owner").get_to(log_entry.log_owner);
}

LogEntry Serializer::serialize(std::string source) {
  json parsed = json::parse(source);
  return parsed;
}
