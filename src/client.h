#ifndef CLIENT_H
#define CLIENT_H

#include <string>

struct Client {
    int client_socket;

    // Client::make_request(request)
    //     Makes request to server. Expects a serialized int in response.
    //     Returns a nonnegative integer on success and -1 otherwise.
    int make_request(const std::string request);
};

#endif
