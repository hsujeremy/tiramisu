#define _XOPEN_SOURCE

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "common.h"
#include "producer_client.h"

int main() {
    ProducerClient* prod = new ProducerClient();
    if (prod->connect_to_server() < 0) {
        perror("Error connecting to server!\n");
        return -1;
    }

    prod->init();
    prod->begin_transaction();
    for (size_t i = 0; i < 10; ++i) {
        prod->send_record(i);
    }
    prod->commit_transaction();
    prod->disconnect_from_server();

    delete prod;
    return 0;
}
