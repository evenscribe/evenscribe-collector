#ifndef SERVICE_MAC_H
#define SERVICE_MAC_H

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

#if defined(__MACH__)

char *populate_plist_path(void);
char *populate_plist(int *length);
void ensure_directory_exists(char *evenscribe_plist_path);
int safe_exec(char *const argv[], bool suppress_output);
int service_install_internal(char *evenscribe_plist_path);
int service_start(void);
int service_stop(void);

#endif
#endif // SERVICE_MAC_H
