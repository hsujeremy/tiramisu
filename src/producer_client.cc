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
    ssize_t nread = read(client_socket, buf, 1024);
    if (nread) {
        dbg_printf(DBG, "From server: %s\n", buf);
    }

    state = UNINITIALIZED;
    dbg_printf(DBG, "Producer connected at socket %d\n", client_socket);
    return 0;
}

void ProducerClient::close_connection() {
    if (client_socket < 0) {
        dbg_printf(DBG, "Invalid client_socket %d\n", client_socket);
        return;
    }

    if (state == DISCONNECTED) {
        dbg_printf(DBG, "Connection already closed!\n");
        return;
    }

    if (close(client_socket) < 0) {
        dbg_printf(DBG, "Failed to close connection\n");
    }
    state = DISCONNECTED;
    dbg_printf(DBG, "Successfully closed connection\n");
}

int ProducerClient::init_transactions() {
    if (client_socket < 0 || state == DISCONNECTED) {
        dbg_printf(DBG, "Client not connected to server\n");
        return -1;
    }

    // Assume for now that there this function is not called twice
    assert(state == UNINITIALIZED);

    id = make_request("init_transactions");
    if (id < 0) {
        dbg_printf(DBG, "Error getting the transactional ID!\n");
        return -1;
    }
    dbg_printf(DBG, "Producer id from server: %d\n", id);
    state = INITIALIZED;
    return id;
}

int ProducerClient::begin_transaction() {
    return make_request("begin_transaction");
}

// "send_record,<topic>,<data>,<event_time>"
int ProducerClient::send_record(const int data, const std::string topic) {
    std::time_t event_time = std::time(nullptr);
    std::string request = "send_record," + topic + "," + std::to_string(data) +
                          "," + std::to_string(event_time);
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
