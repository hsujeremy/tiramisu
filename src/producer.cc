#define _XOPEN_SOURCE

#include <iostream>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "common.h"

int connect_producer() {
  int client_socket;
  size_t len;
  sockaddr_un remote;

  printf("Attempting to connect...\n");
  client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (client_socket == -1) {
    printf("L%d: Failed to create socket!\n", __LINE__);
  }

  remote.sun_family = AF_UNIX;
  strncpy(remote.sun_path, SOCK_PATH, strlen(SOCK_PATH) + 1);
  len = strlen(remote.sun_path) + sizeof(remote.sun_family) + 1;
  if (connect(client_socket, (sockaddr *)&remote, len) == -1) {
    printf("Connection failed\n");
    return -1;
  }

  printf("Producer connected at socket %d\n", client_socket);
  return client_socket;
}

int main() {
  int client_socket = connect_producer();
  if (client_socket < 0) {
    exit(1);
  }

  return 0;
}
