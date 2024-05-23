#include "config.h"
#include "helper.h"
#include "log.h"
#include "service.h"
#include "socket.h"
#include <cstring>
#include <iostream>

#define DEBUG(x) std::cout << '>' << #x << ':' << x << '\n';

#define SERVICE_START_OPT "--start-service"
#define SERVICE_STOP_OPT "--stop-service"

static void parse_arguments(int argc, char **argv) {
  if (string_equals(argv[1], SERVICE_START_OPT)) {
    exit(service_start());
  }
  if (string_equals(argv[1], SERVICE_STOP_OPT)) {
    exit(service_stop());
  }
}

int main(int argc, char **argv) {
  if (!config_file_exstis()) {
    error("Config file doesn't exist : $HOME/.evenscriberc");
    exit(1);
  }

  Config config;
  try {
    config = config_to_tuple();
  } catch (...) {
    error("Error reading config file. Bad schema detected.");
    exit(1);
  }

  if (argc > 1) {
    parse_arguments(argc, argv);
  }

  Socket socket = Socket(config);
  socket.handle_message();
  return 0;
}
