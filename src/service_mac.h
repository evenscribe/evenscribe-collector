#if defined(__APPLE__) && defined(__MACH__)
#ifndef SERVICE_MAC
#define SERVICE_MAC

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

#define MAXLEN 512
#define _PATH_LAUNCHCTL "/bin/launchctl"
#define _NAME_EVENSCRIBE_PLIST "com.evenscribe.app"
#define _PATH_EVENSCRIBE_PLIST "%s/Library/LaunchAgents/com.evenscribe.plist"

#define _EVENSCRIBE_PLIST                                                      \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"                               \
  "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "                    \
  "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"                      \
  "<plist version=\"1.0\">\n"                                                  \
  "<dict>\n"                                                                   \
  "    <key>Label</key>\n"                                                     \
  "    <string>com.evenscribe.app</string>\n"                                  \
  "    <key>ProgramArguments</key>\n"                                          \
  "    <array>\n"                                                              \
  "        <string>%s</string>\n"                                              \
  "    </array>\n"                                                             \
  "    <key>EnvironmentVariables</key>\n"                                      \
  "    <dict>\n"                                                               \
  "        <key>PATH</key>\n"                                                  \
  "        <string>%s</string>\n"                                              \
  "    </dict>\n"                                                              \
  "    <key>RunAtLoad</key>\n"                                                 \
  "    <true/>\n"                                                              \
  "    <key>KeepAlive</key>\n"                                                 \
  "    <dict>\n"                                                               \
  "        <key>SuccessfulExit</key>\n"                                        \
  " 	     <false/>\n"                                                          \
  " 	     <key>Crashed</key>\n"                                                \
  " 	     <true/>\n"                                                           \
  "    </dict>\n"                                                              \
  "    <key>StandardOutPath</key>\n"                                           \
  "    <string>/tmp/evenscribe_%s.out.log</string>\n"                          \
  "    <key>StandardErrorPath</key>\n"                                         \
  "    <string>/tmp/evenscribe_%s.err.log</string>\n"                          \
  "    <key>ProcessType</key>\n"                                               \
  "    <string>Interactive</string>\n"                                         \
  "    <key>Nice</key>\n"                                                      \
  "    <integer>-20</integer>\n"                                               \
  "</dict>\n"                                                                  \
  "</plist>"

//
// NOTE(koekeishiya): A launchd service has the following states:
//
//          1. Installed / Uninstalled
//          2. Active (Enable / Disable)
//          3. Bootstrapped (Load / Unload)
//          4. Running (Start / Stop)
//

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

  int status = posix_spawn(&pid, argv[0], &actions, NULL, argv, NULL);
  if (status)
    return 1;

  while ((waitpid(pid, &status, 0) == -1) && (errno == EINTR)) {
    usleep(1000);
  }

  if (WIFSIGNALED(status)) {
    return 1;
  } else if (WIFSTOPPED(status)) {
    return 1;
  } else {
    return WEXITSTATUS(status);
  }
  return 1;
}

static char *populate_plist_path(void) {
  char *home = getenv("HOME");

  if (!home) {
    error("evenscribe: unable to retrieve home directory! abort..\n");
  }

  int size = strlen(_PATH_EVENSCRIBE_PLIST) - 2 + strlen(home) + 1;
  char *result = (char *)malloc(size);
  if (!result) {
    error("evenscribe: could not allocate memory for plist path! abort..\n");
  }

  memset(result, 0, size);
  snprintf(result, size, _PATH_EVENSCRIBE_PLIST, home);

  return result;
}

static char *populate_plist(int *length) {
  char *user = getenv("USER");
  if (!user) {
    error("evenscribe: 'env USER' not set! abort..\n");
  }

  char *path_env = getenv("PATH");
  if (!path_env) {
    error("evenscribe: 'env PATH' not set! abort..\n");
  }

  char exe_path[4096];
  unsigned int exe_path_size = sizeof(exe_path);
  if (_NSGetExecutablePath(exe_path, &exe_path_size) < 0) {
    error("evenscribe: unable to retrieve path of executable! abort..\n");
  }

  int size = strlen(_EVENSCRIBE_PLIST) - 8 + strlen(exe_path) +
             strlen(path_env) + (2 * strlen(user)) + 1;
  char *result = (char *)malloc(size);
  if (!result) {
    error(
        "evenscribe: could not allocate memory for plist contents! abort..\n");
  }

  memset(result, 0, size);
  snprintf(result, size, _EVENSCRIBE_PLIST, exe_path, path_env, user, user);
  *length = size - 1;

  return result;
}

static inline void ensure_directory_exists(char *evenscribe_plist_path) {
  //
  // NOTE(koekeishiya): Temporarily remove filename.
  // We know the filepath will contain a slash, as
  // it is controlled by us, so don't bother checking
  // the result..
  //

  char *last_slash = strrchr(evenscribe_plist_path, '/');
  *last_slash = '\0';

  if (!directory_exists(evenscribe_plist_path)) {
    mkdir(evenscribe_plist_path, 0755);
  }

  //
  // NOTE(koekeishiya): Restore original filename.
  //

  *last_slash = '/';
}

static int service_install_internal(char *evenscribe_plist_path) {
  int evenscribe_plist_length;
  char *evenscribe_plist = populate_plist(&evenscribe_plist_length);
  ensure_directory_exists(evenscribe_plist_path);

  FILE *handle = fopen(evenscribe_plist_path, "w");
  if (!handle)
    return 1;

  size_t bytes = fwrite(evenscribe_plist, evenscribe_plist_length, 1, handle);
  int result = bytes == 1 ? 0 : 1;
  fclose(handle);

  return result;
}

static int service_start(void) {
  char *evenscribe_plist_path = populate_plist_path();
  if (!file_exists(evenscribe_plist_path)) {
    int result = service_install_internal(evenscribe_plist_path);
    if (result) {
      error("evenscribe: service file '%s' could not be installed! abort..\n");
    }
  }

  char service_target[MAXLEN];
  snprintf(service_target, sizeof(service_target), "gui/%d/%s", getuid(),
           _NAME_EVENSCRIBE_PLIST);

  char domain_target[MAXLEN];
  snprintf(domain_target, sizeof(domain_target), "gui/%d", getuid());

  //
  // NOTE(koekeishiya): Check if service is bootstrapped
  //

  const char *const args[] = {_PATH_LAUNCHCTL, "print", service_target, NULL};
  int is_bootstrapped = safe_exec((char *const *)args, true);

  if (is_bootstrapped != 0) {

    //
    // NOTE(koekeishiya): Service is not bootstrapped and could be disabled.
    // There is no way to query if the service is disabled, and we cannot
    // bootstrap a disabled service. Try to enable the service. This will be
    // a no-op if the service is already enabled.
    //

    const char *const args[] = {_PATH_LAUNCHCTL, "enable", service_target,
                                NULL};
    safe_exec((char *const *)args, false);

    //
    // NOTE(koekeishiya): Bootstrap service into the target domain.
    // This will also start the program **iff* RunAtLoad is set to true.
    //

    const char *const args2[] = {_PATH_LAUNCHCTL, "bootstrap", domain_target,
                                 evenscribe_plist_path, NULL};
    return safe_exec((char *const *)args2, false);
  } else {

    //
    // NOTE(koekeishiya): The service has already been bootstrapped.
    // Tell the bootstrapped service to launch immediately; it is an
    // error to bootstrap a service that has already been bootstrapped.
    //

    const char *const args[] = {_PATH_LAUNCHCTL, "kickstart", service_target,
                                NULL};
    return safe_exec((char *const *)args, false);
  }
}

static int service_stop(void) {
  char *evenscribe_plist_path = populate_plist_path();
  if (!file_exists(evenscribe_plist_path)) {
    error("evenscribe: service file is not installed! abort..\n");
  }

  char service_target[MAXLEN];
  snprintf(service_target, sizeof(service_target), "gui/%d/%s", getuid(),
           _NAME_EVENSCRIBE_PLIST);

  char domain_target[MAXLEN];
  snprintf(domain_target, sizeof(domain_target), "gui/%d", getuid());

  //
  // NOTE(koekeishiya): Check if service is bootstrapped
  //

  const char *const args[] = {_PATH_LAUNCHCTL, "print", service_target, NULL};
  int is_bootstrapped = safe_exec((char *const *)args, true);

  if (is_bootstrapped != 0) {

    //
    // NOTE(koekeishiya): Service is not bootstrapped, but the program
    // could still be running an instance that was started **while the service
    // was bootstrapped**, so we tell it to stop said service.
    //

    const char *const args[] = {_PATH_LAUNCHCTL, "kill", "SIGTERM",
                                service_target, NULL};
    return safe_exec((char *const *)args, false);
  } else {

    //
    // NOTE(koekeishiya): Service is bootstrapped; we stop a potentially
    // running instance of the program and unload the service, making it
    // not trigger automatically in the future.
    //
    // This is NOT the same as disabling the service, which will prevent
    // it from being boostrapped in the future (without explicitly re-enabling
    // it first).
    //

    const char *const args[] = {_PATH_LAUNCHCTL, "bootout", domain_target,
                                evenscribe_plist_path, NULL};
    safe_exec((char *const *)args, false);

    const char *const args2[] = {_PATH_LAUNCHCTL, "disable", service_target,
                                 NULL};
    return safe_exec((char *const *)args2, false);
  }
}

#endif
#endif
