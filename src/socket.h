#ifndef SOCKET_H
#define SOCKET_H

#include "config.h"
#include "log.h"
#include "param.h"
#include "run_clickhouse.h"
#include "run_postgres.h"
#include <deque>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

extern std::deque<connection_t *> conn_queue;
extern pthread_t conn_threads[];
extern pthread_mutex_t conn_mtx;
extern pthread_cond_t conn_cond_var;

class Socket {
public:
  Socket(Config config);
  void handle_message();

private:
  void _sanitize();
  void _bind();
  void _listen();

  Config config;
  int server_socket;
  struct sockaddr_un addr;
};

#endif // SOCKET_H
