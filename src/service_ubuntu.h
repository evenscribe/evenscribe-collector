#if defined(__linux__) || defined(__linux) || defined(linux)
#ifndef SERVICE_UBUNTU
#define SERVICE_UBUNTU

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

#define MAXLEN 512
#define _PATH_SYSTEMCTL "/bin/systemctl"
#define _NAME_EVENSCRIBE_SERVICE "evenscribe.service"
#define _PATH_EVENSCRIBE_SERVICE "%s/.config/systemd/user/evenscribe.service"

#define _EVENSCRIBE_SERVICE                                                    \
  "[Unit]\n"                                                                   \
  "Description=EvenScribe Service\n"                                           \
  "After=network.target\n"                                                     \
  "\n"                                                                         \
  "[Service]\n"                                                                \
  "ExecStart=%s\n"                                                             \
  "Restart=on-failure\n"                                                       \
  "StandardOutput=journal\n"                                                   \
  "StandardError=journal\n"                                                    \
  "\n"                                                                         \
  "[Install]\n"                                                                \
  "WantedBy=multi-user.target\n"

static void create_service_path_linux() {
  char *home = getenv("HOME");
  if (!home) {
    error("evenscribe: 'env HOME' not set! abort..\n");
  }

  char path[256];
  snprintf(path, sizeof(path), "%s/.config/systemd/user/", home);

  if (create_path_if_not_exists(path) != 0) {
    error("evenscribe: could not create path %s! abort..\n", path);
  }
}

static int safe_exec(char *const argv[], bool suppress_output) {
  pid_t pid;
  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init(&actions);

  if (suppress_output) {
    posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null",
                                     O_WRONLY | O_APPEND, 0);
    posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null",
                                     O_WRONLY | O_APPEND, 0);
  }

  int status = posix_spawn(&pid, argv[0], &actions, NULL, argv, environ);
  posix_spawn_file_actions_destroy(&actions);

  if (status != 0) {
    return 1;
  }

  while (waitpid(pid, &status, 0) == -1) {
    if (errno != EINTR) {
      return 1;
    }
  }

  if (WIFSIGNALED(status) || WIFSTOPPED(status)) {
    return 1;
  } else {
    return WEXITSTATUS(status);
  }
}

static char *populate_service(int *length) {
  char exe_path[4096];
  ssize_t exe_path_size =
      readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
  if (exe_path_size == -1) {
    error("evenscribe: unable to retrieve path of executable! abort..\n");
  }
  exe_path[exe_path_size] = '\0';

  int size = strlen(_EVENSCRIBE_SERVICE) - 2 + strlen(exe_path) + 1;
  char *result = (char *)malloc(size);
  if (!result) {
    error("evenscribe: could not allocate memory for service contents! "
          "abort..\n");
  }

  memset(result, 0, size);
  snprintf(result, size, _EVENSCRIBE_SERVICE, exe_path);
  *length = size - 1;

  return result;
}

static int service_install_internal() {
  int evenscribe_service_length;
  char *evenscribe_service = populate_service(&evenscribe_service_length);

  char *home = getenv("HOME");
  if (!home) {
    error("evenscribe: 'env HOME' not set! abort..\n");
  }

  char service_path[256];
  snprintf(service_path, sizeof(service_path), _PATH_EVENSCRIBE_SERVICE, home);

  FILE *handle = fopen(service_path, "w");
  if (!handle)
    return 1;

  size_t bytes =
      fwrite(evenscribe_service, evenscribe_service_length, 1, handle);
  int result = bytes == 1 ? 0 : 1;
  fclose(handle);

  free(evenscribe_service);

  if (result == 0) {
    const char *const args[] = {_PATH_SYSTEMCTL, "--user", "daemon-reload",
                                NULL};
    result = safe_exec((char *const *)args, false);
  }

  return result;
}

static int service_start(void) {
  create_service_path_linux();

  char *home = getenv("HOME");
  if (!home) {
    error("evenscribe: 'env HOME' not set! abort..\n");
  }
  char service_path[256];
  snprintf(service_path, sizeof(service_path), _PATH_EVENSCRIBE_SERVICE, home);

  if (!file_exists(service_path)) {
    int result = service_install_internal();
    if (result) {
      error("evenscribe: service file '%s' could not be installed! abort..\n",
            _PATH_EVENSCRIBE_SERVICE);
    }
  }

  const char *const args[] = {_PATH_SYSTEMCTL, "--user", "start",
                              _NAME_EVENSCRIBE_SERVICE, NULL};
  return safe_exec((char *const *)args, false);
}

static int service_stop(void) {
  char *home = getenv("HOME");
  if (!home) {
    error("evenscribe: 'env HOME' not set! abort..\n");
  }
  char service_path[256];
  snprintf(service_path, sizeof(service_path), _PATH_EVENSCRIBE_SERVICE, home);

  if (!file_exists(service_path)) {
    error("evenscribe: service file is not installed! abort..\n");
  }

  const char *const args[] = {_PATH_SYSTEMCTL, "--user", "stop",
                              _NAME_EVENSCRIBE_SERVICE, NULL};
  return safe_exec((char *const *)args, false);
}

#endif
#endif
