#ifndef CLIENT_H
#define CLIENT_H

#include <string>

struct Client {
    virtual int make_request(const std::string request) = 0;
};

#endif
