#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* Log { */
/*   level; */
/*   timestamp; */
/*   source; */
/*   message; */
/*   context; */
/* }; */

class Serializer {

private:
  static std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
      return "";
    }
    return str.substr(start, end - start + 1);
  }

public:
  static json serialize(std::string source) {
    json parsed = json::parse(trim(source));
    return parsed;
  }
};
