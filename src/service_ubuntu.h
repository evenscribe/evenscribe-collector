#ifndef SERVICE_UBUNTU_H
#define SERVICE_UBUNTU_H

#include "helper.h"
#include "log.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <spawn.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#if defined(__linux__) || defined(__linux) || defined(linux)

int service_start();
int service_stop();

#endif

#endif // SERVICE_UBUNTU_H
