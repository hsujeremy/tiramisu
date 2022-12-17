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

BrokerManager* broker = nullptr;
volatile sig_atomic_t terminate = 0;

void handler(int signum) {
    terminate = 1;
}

ssize_t Server::send_message(int socket, std::string payload) {
    Message m;
    m.payload = (char*) payload.c_str();
    m.length = strlen(m.payload);

    printf("Sending message from server %zu\n", m.length);
    int r = send(socket, &m, sizeof(Message::length), 0);
    // TODO: eventually the client should return something if there isn't space
    // in the buffer
    if (r < 0) {
        dbg_printf(DBG, "Unable to send message header\n");
        return -1;
    }
    printf("about to send payload: %s\n", m.payload);
    r = send(socket, (char*) payload.c_str(), m.length, 0);
    if (r < 0) {
        dbg_printf(DBG, "Unable to send message payload\n");
    }
    return r;
}

ssize_t Server::recv_message(int socket, std::string* payload) {
    // TODO: have clients send over data via messages

    char buf[BUFSIZE] = {0};
    ssize_t nread = read(socket, buf, BUFSIZE);
    if (nread < 0) {
        // Do something
        return -1;
    }
    buf[nread] = '\0';
    std::string serialized(buf);
    *payload = serialized;
    return nread;
}

int main() {
    assert(MAX_PRODUCERS + MAX_CONSUMERS == MAX_CLIENTS);

    broker = new BrokerManager();
    Server* server = new Server();
    broker->server = server;
    server->broker = broker;

    char buf[BUFSIZE];

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

    // Specify maximum amount of pending connections for the master socket
    if (listen(master_socket, MAX_PENDING) < 0) {
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
            int sd = server->client_sockets[i];
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

            dbg_printf(DBG, "New connection with socket fd %d, IP %s, and port "
                       "number %d\n", new_socket, inet_ntoa(addr.sin_addr),
                       ntohs(addr.sin_port));

            [[maybe_unused]] int r = server->send_message(new_socket, message);

            // Add new socket to socket array
            for (size_t i = 0; i < MAX_CLIENTS; ++i) {
                if (!server->client_sockets[i]) {
                    server->client_sockets[i] = new_socket;
                    ClientMetadata metadata;
                    metadata.sock = new_socket;
                    metadata.type = UNSPECIFIED;
                    server->sockfd2client.insert({new_socket, metadata});
                    break;
                }
            }
            continue;
        }

        // Handle IO operation on some other socket
        for (size_t i = 0; i < MAX_CLIENTS; ++i) {
            int sd = server->client_sockets[i];
            if (!FD_ISSET(sd, &readfds)) {
                continue;
            }
            std::string incoming;
            ssize_t nread = server->recv_message(sd, &incoming);
            if (!nread) {
                // If peer disconnected, then close the socket descriptor and
                // clean up the associated producer
                getpeername(sd, (struct sockaddr*) &addr, (socklen_t*) &addrlen);
                dbg_printf(DBG, "Host disconnected with IP %s and port %d\n",
                           inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                close(sd);
                server->cleanup_client(i);
                continue;
            }
            assert(nread <= BUFSIZE);
            strncpy(buf, incoming.c_str(), nread + 1);
            buf[nread] = '\0';
            dbg_printf(DBG, "From client: %s\n", buf);

            // Attempt to satisfy client request
            std::string request(buf);
            RequestedAction action = broker->parse_request(request);
            int result = broker->execute(sd, action, request);
            std::string serialized_result = std::to_string(result);

            // Echo back a response
            server->send_message(sd, serialized_result);
        }
    }

    // TODO: Handle case where server quits before clients
    delete server;
    delete broker;

    return 0;
}
