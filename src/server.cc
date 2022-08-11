#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "broker.h"
#include "common.h"

#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8888
#define MAX_CLIENTS 30

BrokerManager* broker = nullptr;

void Server::setup() {
  if (!broker) {
    printf("BrokerManager not initialized!\n");
    return;
  }

  size_t len;
  sockaddr_un local;

  printf("Setting up server...\n");
  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_socket == -1) {
    printf("Failed to create socket\n");
    return;
  }

  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, SOCK_PATH, strlen(SOCK_PATH) + 1);
  unlink(local.sun_path);

  len = strlen(local.sun_path) + sizeof(local.sun_family) + 1;
  if (bind(server_socket, (sockaddr*)&local, len) == -1) {
    printf("Socket failed to bind\n");
    return;
  }

  if (listen(server_socket, 5) == -1) {
    printf("Failed to listen on socket\n");
  }
}

void Server::handle_client(const int client_socket,
                           const ClientType client_type) {
  if (!broker) {
    printf("BrokerManager not initialized!\n");
    return;
  }

  int prod_idx = -1;
  for (int i = 0; i < MAX_PRODUCERS; ++i) {
    if (!broker->producers[i]) {
      prod_idx = i;
      // Set index in table to be the transactional_id for that producer
      broker->producers[i] = new Producer(client_socket, prod_idx);
    }
  }

  if (prod_idx == -1) {
    printf("No space for producer\n");
    return;
  }

  bool finished = false;

  printf("Connected to socket: %d.\n", client_socket);

  // Create two packets, one from which to read and one from which to receive
  Message send_message;
  Message recv_message;

  // Just echo back client message with OK status for now
  while (!finished) {
    ssize_t length = recv(client_socket, &recv_message, sizeof(Message), 0);
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
      const std::string client_request(recv_message.payload);
      RequestedAction action = broker->parse_request(client_request);

      int result = broker->execute(PRODUCER, action, client_request);
      std::string serialized_result = std::to_string(result);

      send_message.length = serialized_result.length();
      char send_buffer[send_message.length + 1];
      strcpy(send_buffer, serialized_result.c_str());
      send_message.payload = send_buffer;
      send_message.status = OK_DONE;

      // Send status of the received message (OK, UNKNOWN_QUERY, etc)
      int r = send(client_socket, &send_message, sizeof(Message), 0);
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
  delete broker->producers[prod_idx];
  broker->producers[prod_idx] = nullptr;
}

int main() {
  // broker = new BrokerManager();
  // Server* server = new Server();
  // broker->server = server;
  // server->setup();
  // if (server->server_socket < 0) {
  //   exit(1);
  // }
  // printf("Server socket %d waiting for a connection...\n",
  //        server->server_socket);

  // sockaddr_un remote;
  // socklen_t len = sizeof(remote);
  // int client_socket = accept(server->server_socket, (sockaddr*)&remote, &len);
  // if (client_socket == -1) {
  //   printf("Failed to accept a new connection\n");
  //   exit(1);
  // }

  // server->handle_client(client_socket, PRODUCER);
  // delete server;
  // delete broker;
  // return 0;

  broker = new BrokerManager();
  Server* server = new Server();
  broker->server = server;

  int opt = 1;
  int new_socket;
  int client_sockets[MAX_CLIENTS] = {0};
  int activity;
  int sd;
  int max_sd;

  struct sockaddr_in addr;
  char buf[1024];

  // Set of socket descriptors
  fd_set readfds;

  // Some message
  std::string message = "ECHO Daemon v1.0\r\n";

  int master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (master_socket == 0) {
    perror("Failed to create master socket\n");
    exit(EXIT_FAILURE);
  }

  // Set the master socket to allow multiple connections
  int r = setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,
                     sizeof(int));
  if (r < 0) {
    perror("Failed on setsockopt\n");
    exit(EXIT_FAILURE);
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);

  // Bind socket to localhost port 8888
  r = bind(master_socket, (struct sockaddr*)&addr, sizeof(addr));
  if (r < 0) {
    perror("bind failed\n");
    exit(EXIT_FAILURE);
  }
  printf("Listening on port %d\n", PORT);

  // Specify max of 3 pending connections for the master socket at any point
  r = listen(master_socket, 3);
  if (r < 0) {
    perror("listen failed\n");
    exit(EXIT_FAILURE);
  }

  // Accept incoming connections
  size_t addrlen = sizeof(addr);
  while (true) {
    // Clear socket set
    FD_ZERO(&readfds);

    // Add master socket to set
    FD_SET(master_socket, &readfds);
    max_sd = master_socket;

    // Add child sockets to set
    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
      sd = client_sockets[i];
      if (sd > 0) {
        FD_SET(sd, &readfds);
      }
      max_sd = std::max(max_sd, sd);
    }

    // Wait indefinitely for activity from at least one of the sockets
    activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);
    if (activity < 0 && errno != EINTR) {
      perror("select error\n");
    }

    // Handle new connection
    if (FD_ISSET(master_socket, &readfds)) {
      new_socket = accept(master_socket, (struct sockaddr*)&addr,
                          (socklen_t*)&addrlen);
      if (new_socket < 0) {
        perror("accept error\n");
        exit(EXIT_FAILURE);
      }

      printf("New connection with socket fd %d, IP %s, and port number %d\n",
             new_socket, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

      // Create Producer by default for now
      int prod_idx = -1;
      for (int i = 0; i < MAX_PRODUCERS; ++i) {
        if (!broker->producers[i]) {
          prod_idx = i;
          // Set index in table to be the transactional_id for that producer
          broker->producers[i] = new Producer(new_socket, prod_idx);
        }
      }

      r = send(new_socket, message.c_str(), message.length(), 0);
      if (r < 0) {
        perror("Error sending connection message\n");
      }

      // Add new socket to socket array
      for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        if (!client_sockets[i]) {
          client_sockets[i] = new_socket;
          break;
        }
      }
    }

    // Then handle IO operation on some other socket
    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
      sd = client_sockets[i];
      if (FD_ISSET(sd, &readfds)) {
        ssize_t nread = read(sd, buf, 1024);
        if (!nread) {
          getpeername(sd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
          printf("Host disconnected with IP %s and port %d\n",
                 inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
          close(sd);
          client_sockets[i] = 0;
        } else {
          // Echo back the incoming message
          buf[nread] = '\0';
          printf("From client: %s\n", buf);
          std::string request(buf);
          RequestedAction action = broker->parse_request(request);

          int result = broker->execute(PRODUCER, action, request);
          std::string ser_result = std::to_string(result);

          send(sd, ser_result.c_str(), strlen(ser_result.c_str()), 0);
        }
      }
    }
  }

  delete server;
  delete broker;

  return 0;
}
