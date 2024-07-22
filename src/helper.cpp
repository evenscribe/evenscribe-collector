#include "helper.h"

bool file_exists(char *filename) {
  struct stat buffer;
  if (stat(filename, &buffer) != 0) {
    return false;
  }
  if (buffer.st_mode & S_IFDIR) {
    return false;
  }
  return true;
}

bool directory_exists(char *filename) {
  struct stat buffer;
  if (stat(filename, &buffer) != 0) {
    return false;
  }
  return S_ISDIR(buffer.st_mode);
}

bool string_equals(const char *a, const char *b) {
  return a && b && strcmp(a, b) == 0;
}

std::string wrap(const std::string &str, const std::string &wrap) {
  return wrap + str + wrap;
}

std::string between(const std::string &start, const std::string &str,
                    const std::string &end) {
  return start + str + end;
}

std::string between(const std::string &start, const uint64_t &number,
                    const std::string &end) {
  return start + std::to_string(number) + end;
}

std::string commaSeparate(std::deque<std::string> &arr) {
  std::ostringstream oss;
  if (!arr.empty()) {
    oss << arr[0];
    for (size_t i = 1; i < arr.size(); ++i) {
      oss << "," << arr[i];
    }
  }
  return oss.str();
}

int create_directory(const char *path) {
  struct stat st = {0};

  // Check if the directory exists
  if (stat(path, &st) == -1) {
    // Create the directory
    if (mkdir(path, 0755) != 0) {
      perror("mkdir");
      return -1;
    }
  }
  return 0;
}

int create_path_if_not_exists(const char *path) {
  char tmp[256];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", path);
  len = strlen(tmp);
  if (tmp[len - 1] == '/') {
    tmp[len - 1] = 0;
  }
  for (p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = 0;
      if (create_directory(tmp) != 0) {
        return -1;
      }
      *p = '/';
    }
  }
  return create_directory(tmp);
}

bool is_socket_open(int sockfd) {
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
