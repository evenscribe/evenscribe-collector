
#ifndef HELPER_H
#define HELPER_H

#include "param.h"
#include <cstring>
#include <deque>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>

typedef struct {
  int client_socket;
  int thread_id;
} connection_t;

bool file_exists(char *filename);
bool directory_exists(char *filename);
bool string_equals(const char *a, const char *b);
std::string wrap(const std::string &str, const std::string &wrap);
std::string between(const std::string &start, const std::string &str,
                    const std::string &end);
std::string between(const std::string &start, const uint64_t &number,
                    const std::string &end);
std::string commaSeparate(std::deque<std::string> &arr);
int create_directory(const char *path);
int create_path_if_not_exists(const char *path);
bool is_socket_open(int sockfd);

#endif // HELPER_H
