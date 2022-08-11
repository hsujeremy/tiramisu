#include <cassert>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "producer_client.h"

#include <arpa/inet.h>

int ProducerClient::connect_to_server() {
    printf("Attemping to connect to server at port %d\n", PORT);
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
      perror("Error creating socket\n");
      return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 address from text to binary
    int r = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    if (r <= 0) {
      perror("Invalid address! Address not supported\n");
      return -1;
    }

    client_fd = connect(client_socket, (struct sockaddr*)&server_addr,
                        sizeof(struct sockaddr));
    if (client_fd < 0) {
      perror("Connection failed\n");
      return -1;
    }

    state = UNINITIALIZED;
    printf("Producer connected at socket %d\n", client_socket);
    return 0;
}

int ProducerClient::make_request(const std::string request) {
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
  std::string serialized_response = payload;

  int status;
  try {
    status = std::stoi(serialized_response);
  } catch (const std::invalid_argument& ia) {
    printf("Unable to cast string \"%s\" to int\n",
           serialized_response.c_str());
    return -2;
  }
  return status;
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

  transactional_id = make_request("init_transactions");
  if (transactional_id < 0) {
    printf("Error getting the transactional ID!\n");
    return -1;
  }
  printf("transactional_id from server: %d\n", transactional_id);
  state = INITIALIZED;
  return transactional_id;
}

int ProducerClient::begin_transaction() {
  return make_request("begin_transaction");
}

int ProducerClient::send_record(const int data) {
  std::time_t event_time = std::time(nullptr);
  std::string request =
    "send_record," + std::to_string(data) + "," + std::to_string(event_time);
  return make_request(request);
}

int ProducerClient::commit_transaction() {
  return make_request("commit_transaction");
}

int ProducerClient::abort_transaction() {
  return make_request("abort_transaction");
}

int ProducerClient::close_producer() {
  assert(state == INITIALIZED);
  state = UNINITIALIZED;
  return 0;
}
