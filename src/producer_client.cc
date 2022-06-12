#include <cassert>
#include <ctime>
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

int ProducerClient::make_request(const std::string request,
                                 std::string *response) {
  char buf[request.length() + 1];
  strcpy(buf, request.c_str());

  Message send_message;
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
  *response = payload;
  return 0;
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
  state = DISCONNECTED;
  printf("Successfully closed connection\n");
}

int ProducerClient::init_transactions() {
  if (client_socket < 0 || state == DISCONNECTED) {
    printf("Client not connected to server\n");
    return -1;
  }

  // Assume for now that there this function is not called twice
  assert(state == UNINITIALIZED);

  std::string serialized_txid;
  make_request("init_transactions", &serialized_txid);
  printf("transactional_id from server: %s\n", serialized_txid.c_str());

  // TODO: Handle case where `serialized_txid` cannot be cleanly casted to int
  transactional_id = std::stoi(serialized_txid);
  state = INITIALIZED;
  return transactional_id;
}

void ProducerClient::begin_transaction() {
  std::string response_status;
  make_request("begin_transaction", &response_status);
  (void)response_status;
}

void ProducerClient::send_record(int data) {
  std::string response_status;
  std::time_t event_time = std::time(nullptr);
  std::string request =
    "send_record," + std::to_string(data) + "," + std::to_string(event_time);
  make_request(request, &response_status);
  (void)response_status;
}

void ProducerClient::close_producer() {
  assert(state == INITIALIZED);
  state = UNINITIALIZED;
}
