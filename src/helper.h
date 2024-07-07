#ifndef HELPER
#define HELPER

#include "param.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>

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

static inline char *wrap(const char *str, const char *wrap) {
  size_t wrap_len = strlen(wrap);
  size_t str_len = strlen(str);
  size_t total_len = 2 * wrap_len + str_len + 1; // +1 for the null terminator

  char *result = (char *)malloc(total_len);
  if (!result)
    return nullptr;

  strcpy(result, wrap);
  strcat(result, str);
  strcat(result, wrap);

  return result;
}

static inline std::string wrap(const std::string &str,
                               const std::string &wrap) {
  return wrap + str + wrap;
}

static inline char *between(const char *start, const char *str,
                            const char *end) {
  size_t start_len = strlen(start);
  size_t str_len = strlen(str);
  size_t end_len = strlen(end);
  size_t total_len =
      start_len + str_len + end_len + 1; // +1 for the null terminator

  char *result = (char *)malloc(total_len);
  if (!result)
    return nullptr;

  strcpy(result, start);
  strcat(result, str);
  strcat(result, end);

  return result;
}

static inline std::string between(const std::string &start,
                                  const std::string &str,
                                  const std::string &end) {
  return start + str + end;
}

// Function to concatenate a C-string, a number, and another C-string
static inline char *between(const char *start, const uint &number,
                            const char *end) {
  size_t start_len = strlen(start);
  size_t end_len = strlen(end);

  // Calculate the length needed for the number as a string
  char number_str[21]; // Enough to hold all numbers up to 64-bit unsigned
                       // integer
  snprintf(number_str, sizeof(number_str), "%u", number);
  size_t number_len = strlen(number_str);

  size_t total_len =
      start_len + number_len + end_len + 1; // +1 for the null terminator

  char *result = (char *)malloc(total_len);
  if (!result)
    return nullptr;

  strcpy(result, start);
  strcat(result, number_str);
  strcat(result, end);

  return result;
}

static inline std::string between(const std::string &start,
                                  const uint64_t &number,
                                  const std::string &end) {
  return start + std::to_string(number) + end;
}

static inline char *commaSeparate(const char **arr, size_t arr_size) {
  if (arr_size == 0)
    return nullptr;

  // Calculate the total length needed
  size_t total_len = 0;
  for (size_t i = 0; i < arr_size; ++i) {
    total_len += strlen(arr[i]);
    if (i != arr_size - 1) {
      total_len += 1; // For the comma
    }
  }
  total_len += 1; // For the null terminator

  // Allocate memory for the result
  char *result = (char *)malloc(total_len);
  if (!result)
    return nullptr;

  // Copy the first element
  strcpy(result, arr[0]);

  // Append the remaining elements with commas
  for (size_t i = 1; i < arr_size; ++i) {
    strcat(result, ",");
    strcat(result, arr[i]);
  }

  return result;
}

static inline std::string commaSeparate(std::deque<std::string> &arr) {
  std::ostringstream oss;
  if (!arr.empty()) {
    oss << arr[0];
    for (size_t i = 1; i < arr.size(); ++i) {
      oss << "," << arr[i];
    }
  }
  return oss.str();
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
