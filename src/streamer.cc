#include <iostream>

#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "common.h"

int setup_server() {
  int server_socket;
  size_t len;
  sockaddr_un local;

  printf("Setting up server...\n");
  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_socket == -1) {
    printf("Failed to create socket\n");
    return -1;
  }

  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, SOCK_PATH, strlen(SOCK_PATH) + 1);
  unlink(local.sun_path);

  len = strlen(local.sun_path) + sizeof(local.sun_family) + 1;
  if (bind(server_socket, (struct sockaddr *)&local, len) == -1) {
    printf("Socket failed to bind\n");
    return -1;
  }

  if (listen(server_socket, 5) == -1) {
    printf("Failed to listen on socket\n");
    return -1;
  }

  return server_socket;
}

int main() {
  std::cout << "Hello World!\n";
  int server_socket = setup_server();
  if (server_socket < 0) {
    exit(1);
  }
  printf("Server socket %d waiting for a connection...\n", server_socket);
  return 0;
}
