#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUF_SIZE 100
#define SV_SOCK_PATH "/tmp/unixsock"

struct Listener {
  struct sockaddr_un addr;
};

struct Listener listener;

