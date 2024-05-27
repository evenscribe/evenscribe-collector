#ifndef SERIALIZER
#define SERIALIZER

#include <string>
#include <unordered_map>

struct Log {
  std::string Timestamp; // DateTime64
  std::string TraceId;
  std::string SpanId;
  uint32_t TraceFlags;
  std::string SeverityText;
  int32_t SeverityNumber;
  std::string ServiceName;
  std::string Body;
  std::unordered_map<std::string, std::string> ResourceAttributes;
  std::unordered_map<std::string, std::string> LogAttributes;
};

class Serializer {
public:
  static Log serialize(std::string source);
};

#endif // !SERIALIZER
