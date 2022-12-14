#include <arpa/inet.h>
#include <assert.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "common.h"

int Client::connect_to_server() {
    dbg_printf(DBG, "Attemping to connect to server at port %d\n", PORT);
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating socket\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address! Address not supported\n");
        return -1;
    }

    if (connect(client_socket, (struct sockaddr*)&server_addr,
                sizeof(struct sockaddr)) < 0) {
        perror("Connection failed\n");
        return -1;
    }

    char buf[1024] = {0};
    Message m;
    ssize_t nread = recv(client_socket, &m, sizeof(Message::length), 0);
    if (nread <= 0) {
        dbg_printf(DBG, "Error getting header from server\n");
        return -1;
    }

    size_t nbytes = m.length;
    char payload[nbytes + 1];
    nread = recv(client_socket, payload, nbytes, 0);
    (void) nread;

    state = UNINITIALIZED;
    dbg_printf(DBG, "Producer connected at socket %d\n", client_socket);
    return 0;
}

int Client::make_request(const std::string request) {
    // TODO: Check server connection beforehand
    char buf[1024] = {0};

    if (send(client_socket, request.c_str(), request.length(), 0) < 0) {
        perror("Error sending to server\n");
        return -1;
    }

    // TODO: Wait for two responses from the server
    // new two-request scheme from server
    Message m;
    ssize_t nread = recv(client_socket, &m, sizeof(Message::length), 0);
    if (nread <= 0) {
        dbg_printf(DBG, "Error getting header from server\n");
        return -1;
    }

    // TODO: Move this into a general client::recv_message helper function
    printf("buf %zu\n", m.length);
    size_t nbytes = m.length;
    char payload[nbytes + 1];
    nread = recv(client_socket, payload, nbytes, 0);
    assert(nread == nbytes);
    payload[nbytes] = '\0';

    std::string response(payload);
    dbg_printf(DBG, "From server: %s\n", payload);

    // TODO: Modify to expect values other than ints
    int status;
    try {
        status = std::stoi(response);
    } catch (const std::invalid_argument& ia) {
        dbg_printf(DBG, "Unable to cast string \"%s\" to int\n", response.c_str());
        return -2;
    }
    return status;
}

int Client::disconnect_from_server() {
    if (client_socket < 0) {
        dbg_printf(DBG, "Invalid client_socket %d\n", client_socket);
        return -1;
    }

    if (state == DISCONNECTED) {
        dbg_printf(DBG, "Connection already closed!\n");
        return -1;
    }

    if (close(client_socket) < 0) {
        dbg_printf(DBG, "Failed to close connection\n");
        return -1;
    }
    state = DISCONNECTED;
    dbg_printf(DBG, "Successfully closed connection\n");
    return 0;
}
