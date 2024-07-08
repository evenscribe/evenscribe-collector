#include "config.h"
#include "helper.h"
#include "log.h"
#include "socket.h"

#ifdef __APPLE__
#include "service.h"

#define SERVICE_START_OPT "--start-service"
#define SERVICE_STOP_OPT "--stop-service"

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

#ifdef __APPLE__
  if (argc > 1) {
    parse_arguments(argc, argv);
  }
#endif
  Config config = config_to_tuple();

  Socket socket = Socket(config);
  socket.handle_message();
  return 0;
}
