#ifndef CLIENT_H
#define CLIENT_H

#include <string>

enum ClientState {
    DISCONNECTED,       // Not connected to the server
    UNINITIALIZED,      // Connected but uninitialized
    INITIALIZED,        // Connected and initialized
};

struct Client {
    int client_socket;
    ClientState state = DISCONNECTED;

    // ProducerClient::connect_to_server()
    //     Attempts to connect to the server and sets the client_socket number
    //     if connected. Sets the connection state to UNITIALIZED.
    //     Returns 0 on success and -1 otherwise.
    int connect_to_server();

    virtual int init() = 0;

    // Client::make_request(request)
    //     Makes request to server. Expects a serialized int in response.
    //     Returns a nonnegative integer on success and -1 otherwise.
    int make_request(const std::string request);

    // ProducerClient::close_connection()
    //     Closes the connection the server.
    int disconnect_from_server();
};

#endif
