#ifndef SERIALIZER
#define SERIALIZER

#include "json.h"
#include "log.h"
#include "param.h"
#include <string>
#include <unordered_map>

struct Log {
  uint32_t Timestamp;
  std::string TraceId;
  std::string SpanId;
  uint32_t TraceFlags;
  std::string SeverityText;
  int32_t SeverityNumber;
  std::string ServiceName;
  std::string Body;
  std::unordered_map<std::string, std::string> ResourceAttributes;
  std::unordered_map<std::string, std::string> LogAttributes;
  bool is_vaild; // phantom_data to check if the struct has been
                 // initialized and valid
};

class Serializer {
public:
  static Log serialize(char *source);
};

#endif // !SERIALIZER
