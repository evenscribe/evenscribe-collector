#ifndef SERVICE_MAC_H
#define SERVICE_MAC_H

#if defined(__MACH__)

#include "helper.h"
#include "log.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mach-o/dyld.h>
#include <spawn.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int service_start(void);
int service_stop(void);

#endif
#endif // SERVICE_MAC_H
