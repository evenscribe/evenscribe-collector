#ifndef HELPER
#define HELPER

#include "log.h"
#include "param.h"
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

static inline bool file_exists(char *filename) {
  struct stat buffer;
  if (stat(filename, &buffer) != 0) {
    return false;
  }
  if (buffer.st_mode & S_IFDIR) {
    return false;
  }
  return true;
}


static inline bool directory_exists(char *filename) {
  struct stat buffer;
  if (stat(filename, &buffer) != 0) {
    return false;
  }
  return S_ISDIR(buffer.st_mode);
}

static inline bool string_equals(const char *a, const char *b) {
  return a && b && strcmp(a, b) == 0;
}

#endif // !HELPER
