#include <cassert>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "producer_client.h"

void ProducerClient::connect_to_server() {
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
    return;
  }

  state = UNINITIALIZED;
  printf("Producer connected at socket %d\n", client_socket);
}

void ProducerClient::close_connection() {
  if (client_socket < 0) {
    printf("Invalid client_socket %d\n", client_socket);
    return;
  }

  if (state == DISCONNECTED) {
    printf("Connection already closed!\n");
    return;
  }

  if (close(client_socket) < 0) {
    printf("Failed to close connection\n");
  }
  printf("Successfully closed connection\n");
}

void ProducerClient::init_transactions() {
  if (client_socket < 0 || state == DISCONNECTED) {
    printf("Client not connected to server\n");
    return;
  }

  // Assume for now that there this function is not called twice
  assert(state == UNINITIALIZED);

  // Make a request to the server to set up the transactional_id
  // The server will either return the transactional_id or -1 on failure
}
