#ifndef PERSISTENCE
#define PERSISTENCE

#include <string>

class Persistence {
public:
  virtual void save(std::string query) const = 0;
};
#endif // !PERSISTENCE
