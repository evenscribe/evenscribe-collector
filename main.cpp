#include <clickhouse/client.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFFER_SIZE 100
#define SOCKET_PATH "/tmp/olympus_socket.sock"

using namespace clickhouse;

int main(int argc, char *argv[]) {
  Client client(ClientOptions().SetHost("localhost"));

  struct sockaddr_un addr;

  int socket_file_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);

  if (socket_file_descriptor == -1) {
    fprintf(stderr, "Failed to open socket '%d': %s\n", socket_file_descriptor,
            strerror(errno));
  }

  if (strlen(SOCKET_PATH) > sizeof(addr.sun_path) - 1) {
    fprintf(stderr, "SOCKET_PATH is too long '%lu': %s\n", strlen(SOCKET_PATH),
            strerror(errno));
  }
  if (remove(SOCKET_PATH) == -1 && errno != ENOENT) {
    fprintf(stderr, "SOCKET_PATH is taken and couldn't be removed '%s': %s\n",
            SOCKET_PATH, strerror(errno));
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

  if (bind(socket_file_descriptor, (struct sockaddr *)&addr,
           sizeof(struct sockaddr_un)) == -1) {
    fprintf(stderr, "Failed to bind to socket '%s': %s\n", SOCKET_PATH,
            strerror(errno));
  }

  if (listen(socket_file_descriptor, 5) == -1) {
    fprintf(stderr, "Failed to listen on socket '%d': %s\n",
            socket_file_descriptor, strerror(errno));
  }

  ssize_t numRead;
  char buf[BUFFER_SIZE];
  for (;;) {
    printf("Waiting to accept a connection...\n");
    int cfd = accept(socket_file_descriptor, NULL, NULL);
    printf("Accepted socket fd = %d\n", cfd);

    while ((numRead = read(cfd, buf, BUFFER_SIZE)) > 0) {
      {
        Block block;

        auto name = std::make_shared<ColumnString>();
        name->Append(buf);

        block.AppendColumn("message", name);

        client.Insert("default.my_first_table", block);
      }

      if (write(STDOUT_FILENO, buf, numRead) != numRead) {
        fprintf(stderr, "%s", "ERROR: Cannot write");
      }
    }

    if (numRead == -1) {
      fprintf(stderr, "%s", "ERROR: Cannot read");
    }

    if (close(cfd) == -1) {
      fprintf(stderr, "%s", "ERROR: Cannot read");
    }
  }
}
