#include <cassert>
#include <ctime>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "producer_client.h"

#include <arpa/inet.h>


int ProducerClient::init_producer() {
    if (client_socket < 0 || state == DISCONNECTED) {
        dbg_printf(DBG, "Client not connected to server\n");
        return -1;
    }

    // Assume for now that there this function is not called twice
    assert(state == UNINITIALIZED);

    id = make_request("init_producer");
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
