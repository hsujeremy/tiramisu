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

int ProducerClient::init_transactions() {
  if (client_socket < 0 || state == DISCONNECTED) {
    printf("Client not connected to server\n");
    return -1;
  }

  // Assume for now that there this function is not called twice
  assert(state == UNINITIALIZED);

  // Make a request to the server to set up the transactional_id
  // The server will either return the transactional_id or -1 on failure
  Message send_message;
  std::string request = "init_transactions";
  char buf[request.length() + 1];
  strcpy(buf, request.c_str());
  send_message.payload = buf;
  send_message.status = UNCATEGORIZED;
  send_message.length = strlen(send_message.payload);

  // First sent header to the server
  int r = send(client_socket, &send_message, sizeof(Message), 0);
  if (r == -1) {
    printf("Failed to send message header\n");
    return -1;
  }

  // If header is sent successfully, then send the payload
  r = send(client_socket, send_message.payload, send_message.length, 0);
  if (r == -1) {
    printf("Failed to send message payload\n");
    return -1;
  }

  Message recv_message;
  ssize_t len = recv(client_socket, &recv_message, sizeof(Message), 0);
  if (len == 0) {
    printf("Server closed connection\n");
    return -1;
  } else if (len < 0) {
    printf("Error setting the transactional ID on the server\n");
    return -1;
  }

  if (recv_message.status != OK_DONE || recv_message.length == 0) {
    printf("Server connection error\n");
    return -1;
  }

  unsigned nbytes = recv_message.length;
  char payload[nbytes + 1];
  len = recv(client_socket, payload, nbytes, 0);
  if (!len) {
    printf("Server connection error\n");
    return -1;
  }

  payload[nbytes] = '\0';
  printf("transactional_id from server: %s\n", payload);
  // TODO: Handle the case where payload cannot be cleanly casted
  transactional_id = std::stoi(payload);
  return transactional_id;
}
