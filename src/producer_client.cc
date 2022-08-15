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
        printf("From server: %s\n", buf);
    }

    state = UNINITIALIZED;
    printf("Producer connected at socket %d\n", client_socket);
    return 0;
}

int ProducerClient::make_request(const std::string request) {
    // TODO: Check server connection beforehand
    char buf[1024] = {0};

    if (send(client_socket, request.c_str(), request.length(), 0) < 0) {
        perror("Error sending to server\n");
        return -1;
    }

    ssize_t nread = read(client_socket, buf, 1024);
    if (!nread) {
        perror("Server connection error\n");
        return -1;
    }

    buf[nread] = '\0';
    std::string response(buf);
    printf("From server: %s\n", response.c_str());

    int status;
    try {
        status = std::stoi(response);
    } catch (const std::invalid_argument& ia) {
        printf("Unable to cast string \"%s\" to int\n", response.c_str());
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

    id = make_request("init_transactions");
    if (id < 0) {
        printf("Error getting the transactional ID!\n");
        return -1;
    }
    printf("Producer id from server: %d\n", id);
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
