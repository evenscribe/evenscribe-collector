#include "service.h"
#include "socket.h"
#include <cstring>

#define SERVICE_START_OPT "--start-service"
#define SERVICE_STOP_OPT "--stop-service"

static inline bool string_equals(const char *a, const char *b) {
  return a && b && strcmp(a, b) == 0;
}

static void parse_arguments(int argc, char **argv) {
  if (string_equals(argv[1], SERVICE_START_OPT)) {
    exit(service_start());
  }

  if (string_equals(argv[1], SERVICE_STOP_OPT)) {
    exit(service_stop());
  }
}

int main(int argc, char **argv) {
  if (argc > 1) {
    parse_arguments(argc, argv);
  }

  Socket socket = Socket();
  socket.handle_message();
  return 0;
}
