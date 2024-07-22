#ifndef CLICKHOUSE_HANDLER_H
#define CLICKHOUSE_HANDLER_H

#include "config.h"
#include "helper.h"
#include "log.h"
#include "param.h"
#include "query_generator_clickhouse.h"
#include <clickhouse/client.h>
#include <deque>
#include <pthread.h>
#include <unistd.h>

extern pthread_t conn_threads[CONN_THREADS];
extern std::deque<connection_t *> conn_queue;
extern pthread_mutex_t conn_mtx;
extern pthread_cond_t conn_cond_var;

namespace Clickhouse {

extern std::deque<char *> read_queue;
extern pthread_t read_threads[READ_THREADS];
extern pthread_mutex_t read_mtx;
extern pthread_cond_t read_cond_var;

extern std::deque<std::string> insert_statements;
extern pthread_t write_threads[WRITE_THREADS];
extern pthread_mutex_t write_mtx;
extern pthread_cond_t write_cond_var;

extern pthread_t sync_thread;

extern clickhouse::Client *db;

void run(Config config);

} // namespace Clickhouse

#endif // CLICKHOUSE_HANDLER_H
