#ifndef HELPER
#define HELPER

#include "param.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <vector>

typedef struct {
  int client_socket;
  int thread_id;
} connection_t;

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

static inline std::string wrap(const std::string &str,
                               const std::string &wrap) {
  return wrap + str + wrap;
}

static inline std::string between(const std::string &start,
                                  const std::string &str,
                                  const std::string &end) {
  return start + str + end;
}

static inline std::string between(const std::string &start,
                                  const uint64_t &number,
                                  const std::string &end) {
  return start + std::to_string(number) + end;
}

static inline std::string commaSeparate(const std::vector<std::string> &arr) {
  std::ostringstream oss;
  if (!arr.empty()) {
    oss << arr[0];
    for (size_t i = 1; i < arr.size(); ++i) {
      oss << "," << arr[i];
    }
  }
  return oss.str();
}

static inline std::string
get_query_from_bucket(std::vector<std::tuple<std::string, time_t>> bucket) {
  std::vector<std::string> query_strings;
  query_strings.reserve(bucket.size());
  for (const auto &t : bucket) {
    query_strings.push_back(std::get<0>(t));
  }
  std::ostringstream query_string;
  query_string << between("INSERT INTO ", TABLE_NAME, " VALUES ")
               << commaSeparate(query_strings) << ";";
  return query_string.str();
}

static bool is_socket_open(int sockfd) {
  char buffer;
  ssize_t result = recv(sockfd, &buffer, 1, MSG_PEEK);

  if (result == 0) {
    return false;
  } else if (result < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return true;
    } else {
      return false;
    }
  }

  return true;
}

#endif // !HELPER
