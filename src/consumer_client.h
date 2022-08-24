#ifndef CONSUMER_H
#define CONSUMER_H

#include <string>
#include "client.h"
#include "common.h"

struct ConsumerClient : Client {
    int id;

    virtual int init();
    int subscribe(const std::string topic);
    int unsubscribe(const std::string topic);
};

#endif
