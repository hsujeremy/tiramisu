#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "common.h"

int Client::make_request(const std::string request) {
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
    dbg_printf(DBG, "From server: %s\n", response.c_str());

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
