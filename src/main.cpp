#include "config.h"
#include "helper.h"
#include "log.h"
#include "socket.h"

#define SERVICE_START_OPT "--start-service"
#define SERVICE_STOP_OPT "--stop-service"

#if defined(__APPLE__) && defined(__MACH__)
#include "service.h"

static void parse_arguments(int argc, char **argv) {
  if (string_equals(argv[1], SERVICE_START_OPT)) {
    exit(service_start());
  }
  if (string_equals(argv[1], SERVICE_STOP_OPT)) {
    exit(service_stop());
  }
  error("evenscribe(command): invalid argument passed");
}
#endif

#if defined(__linux__) || defined(__linux) || defined(linux)
#include "service_ubuntu.h"

static void parse_arguments(int argc, char **argv) {
  if (string_equals(argv[1], SERVICE_START_OPT)) {
    exit(service_start());
  }
  if (string_equals(argv[1], SERVICE_STOP_OPT)) {
    exit(service_stop());
  }
  error("evenscribe(command): invalid argument passed");
}
#endif

int main(int argc, char **argv) {

#if defined(__APPLE__) && defined(__MACH__)
  if (argc > 1) {
    parse_arguments(argc, argv);
  }
#endif

#if defined(__linux__) || defined(__linux) || defined(linux)
  create_service_path_linux();
  if (argc > 1) {
    parse_arguments(argc, argv);
  }
#endif

  Config config = config_to_tuple();

  Socket socket = Socket(config);
  socket.handle_message();
  return 0;
}
