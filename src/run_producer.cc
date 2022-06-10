#define _XOPEN_SOURCE

#include <iostream>
#include <cassert>

#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "common.h"

#define STDIN_BUFSIZE 1024

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

  Packet send_message;
  Packet recv_message;

  // Continuously loop and wait for user input. At each iteration, output an
  // interactive marker and read from stdin until EOF
  char read_buffer[STDIN_BUFSIZE];
  send_message.payload = read_buffer;
  send_message.status = UNCATEGORIZED;
  char *output = nullptr;
  while (printf("%s", "client > "),
         output = fgets(read_buffer, STDIN_BUFSIZE, stdin),
         !feof(stdin)) {
    if (!output) {
      printf("fgets failed\n");
      break;
    }

    send_message.payload = read_buffer;
    send_message.length = strlen(read_buffer);

    if (send_message.length > 1) {
      // First send header to the server
      int r = send(client_socket, &send_message, sizeof(Packet), 0);
      if (r == -1) {
        printf("Failed to send message header\n");
        exit(1);
      }

      // If header is sent successfully, then send the payload
      r = send(client_socket, send_message.payload, send_message.length, 0);
      if (r == -1) {
        printf("Failed to send message payload\n");
        exit(1);
      }

      // Always wait for a response from the server, even if it just an OK
      ssize_t len = recv(client_socket, &recv_message, sizeof(Packet), 0);
      if (len > 0) {
        if (recv_message.status == OK_DONE && recv_message.length > 0) {
          unsigned nbytes = recv_message.length;
          char payload[nbytes + 1];
          len = recv(client_socket, payload, nbytes, 0);
          if (len > 0) {
            payload[nbytes] = '\0';
            printf("%s\n", payload);
          }
        }
      } else {
        if (len < 0) {
          printf("Failed to receive message\n");
        } else {
          printf("Server closed connection\n");
        }
        exit(1);
      }
    }
  }

  close(client_socket);
  return 0;
}
