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
  std::string message = "ECHO Daemon v1.0\r";

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

    // Either handle a new connection or some IO operation on an existing one
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
          server->sd_client_map.insert(
            std::make_pair<size_t, size_t>(new_socket, i)
          );
          printf("Created producer with id %d\n", prod_idx);
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
    } else {
      // Handle IO operation on some other socket
      for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        sd = client_sockets[i];
        if (FD_ISSET(sd, &readfds)) {
          ssize_t nread = read(sd, buf, 1024);
          if (!nread) {
            // If peer disconnected, then close the socket descriptor and clean
            // up the associated producer
            getpeername(sd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
            printf("Host disconnected with IP %s and port %d\n",
                   inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            close(sd);
            Producer* exited_prod =
              broker->producers[server->sd_client_map.at(sd)];
            printf("Exited producer with id %d\n",
                   exited_prod->transactional_id);
            server->sd_client_map.erase(sd);
            client_sockets[i] = 0;
            broker->producers[exited_prod->transactional_id] = nullptr;
            delete exited_prod;
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
  }

  delete server;
  delete broker;

  return 0;
}
