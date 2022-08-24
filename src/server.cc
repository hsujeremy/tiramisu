#include <cassert>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include "broker.h"
#include "common.h"

#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define MAX_CLIENTS 30

BrokerManager* broker = nullptr;
volatile sig_atomic_t terminate = 0;

void handler(int signum) {
    terminate = 1;
}

int main() {
    broker = new BrokerManager();
    Server* server = new Server();
    broker->server = server;

    int client_sockets[MAX_CLIENTS] = {0};
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
    int opt = 1;
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,
                   sizeof(int)) < 0) {
        perror("Failed on setsockopt\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    // Bind socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }
    dbg_printf(DBG, "Listening on port %d\n", PORT);

    // Specify max of 3 pending connections for the master socket at any point
    if (listen(master_socket, 3) < 0) {
        perror("listen failed\n");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections
    signal(SIGINT, handler);
    size_t addrlen = sizeof(addr);
    while (true) {
        // Clear socket set
        FD_ZERO(&readfds);

        // Add master socket to set
        FD_SET(master_socket, &readfds);
        int max_sd = master_socket;

        // Add child sockets to set
        for (size_t i = 0; i < MAX_CLIENTS; ++i) {
            int sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            max_sd = std::max(max_sd, sd);
        }

        // Wait indefinitely for activity from at least one of the sockets
        // while there's no keyboard interrupt
        if (!terminate
            && select(max_sd + 1, &readfds, nullptr, nullptr, nullptr) < 0
            && errno != EINTR) {
            perror("select error\n");
        }

        if (terminate) {
            break;
        }

        // Either handle a new connection or some IO operation on an existing
        // one
        if (FD_ISSET(master_socket, &readfds)) {
            int new_socket = accept(master_socket, (struct sockaddr*)&addr,
                                    (socklen_t*)&addrlen);
            if (new_socket < 0) {
                perror("accept error\n");
                exit(EXIT_FAILURE);
            }

            dbg_printf(DBG, "New connection with socket fd %d, IP %s, and port number %d\n",
                       new_socket, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

            if (send(new_socket, message.c_str(), message.length(), 0) < 0) {
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
                int sd = client_sockets[i];
                if (FD_ISSET(sd, &readfds)) {
                    ssize_t nread = read(sd, buf, 1024);
                    if (!nread) {
                        // If peer disconnected, then close the socket
                        // descriptor and clean up the associated producer
                        getpeername(sd, (struct sockaddr*)&addr,
                                    (socklen_t*)&addrlen);
                        dbg_printf(DBG, "Host disconnected with IP %s and port %d\n",
                            inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                        close(sd);
                        Producer* exited_prod =
                            broker->producers[server->sd_producer_map.at(sd)];
                        dbg_printf(DBG, "Exited producer with id %d\n", exited_prod->id);
                        server->sd_producer_map.erase(sd);
                        client_sockets[i] = 0;
                        broker->producers[exited_prod->id] = nullptr;
                        delete exited_prod;
                    } else {
                        // Echo back the incoming message
                        buf[nread] = '\0';
                        dbg_printf(DBG, "From client: %s\n", buf);
                        std::string request(buf);
                        RequestedAction action = broker->parse_request(request);

                        int result =
                            broker->execute(PRODUCER, sd, action, request);
                        std::string serialized_result = std::to_string(result);
                        send(sd, serialized_result.c_str(),
                             serialized_result.length(), 0);
                    }
                }
            }
        }
    }

    delete server;
    delete broker;

    return 0;
}
