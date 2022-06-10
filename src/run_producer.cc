#define _XOPEN_SOURCE

#include <iostream>
#include <cassert>

#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "common.h"
#include "producer.h"

#define STDIN_BUFSIZE 1024

int main() {
  Producer *prod = new Producer;
  int client_socket = prod->connect_to_server();
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
