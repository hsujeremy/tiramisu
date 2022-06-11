#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "broker.h"
#include "common.h"

Broker *broker = nullptr;

int setup_server() {
  if (!broker) {
    printf("Broker not initialized!\n");
    return -1;
  }

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
  if (bind(server_socket, (sockaddr *)&local, len) == -1) {
    printf("Socket failed to bind\n");
    return -1;
  }

  if (listen(server_socket, 5) == -1) {
    printf("Failed to listen on socket\n");
    return -1;
  }

  return server_socket;
}

void handle_client(int client_socket) {
  if (!broker) {
    printf("Broker not initialized!\n");
    return;
  }

  int prod_idx = -1;
  for (int i = 0; i < MAX_PRODUCERS; ++i) {
    if (!broker->producers[i]) {
      prod_idx = i;
      broker->producers[i] = new ProducerMetadata(client_socket);
    }
  }

  if (prod_idx == -1) {
    printf("No space for producer\n");
    return;
  }

  bool finished = false;

  printf("Connected to socket: %d.\n", client_socket);

  // Create two packets, one from which to read and one from which to receive
  Packet send_message;
  Packet recv_message;

  // Just echo back client message with OK status for now
  while (!finished) {
    ssize_t length = recv(client_socket, &recv_message, sizeof(Packet), 0);
    if (length < 0) {
      printf("Client connection closed!\n");
      exit(1);
    } else if (length == 0) {
      finished = true;
    }

    if (!finished) {
      char recv_buffer[recv_message.length + 1];
      length = recv(client_socket, recv_buffer, recv_message.length, 0);
      recv_message.payload = recv_buffer;
      recv_message.payload[recv_message.length] = '\0';

      // Now start to parse the message
      std::string client_request(recv_message.payload);
      RequestedAction action = broker->parse_request(client_request);
      int result = broker->execute(action);
      std::string serialized_result = std::to_string(result);

      send_message.length = serialized_result.length();
      char send_buffer[send_message.length + 1];
      strcpy(send_buffer, serialized_result.c_str());
      send_message.payload = send_buffer;
      send_message.status = OK_DONE;

      // Send status of the received message (OK, UNKNOWN_QUERY, etc)
      int r = send(client_socket, &send_message, sizeof(Packet), 0);
      if (r == -1) {
          printf("Failed to send message.");
          exit(1);
      }

      // Send response to the request
      r = send(client_socket, send_message.payload, send_message.length, 0);
      if (r == -1) {
          printf("Failed to send message.");
          exit(1);
      }
    }
  }

  printf("Connection closed at socket %d\n", client_socket);
  close(client_socket);
}

int main() {
  broker = new Broker();

  int server_socket = setup_server();
  if (server_socket < 0) {
    exit(1);
  }
  printf("Server socket %d waiting for a connection...\n", server_socket);

  sockaddr_un remote;
  socklen_t len = sizeof(remote);
  int client_socket = accept(server_socket, (sockaddr *)&remote, &len);
  if (client_socket == -1) {
    printf("Failed to accept a new connection\n");
    exit(1);
  }

  handle_client(client_socket);
  return 0;
}
