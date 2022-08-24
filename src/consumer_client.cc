#include <cassert>
#include "consumer_client.h"

int ConsumerClient::init() {
    if (client_socket < 0 || state == DISCONNECTED) {
        dbg_printf(DBG, "Client not connected to server\n");
        return -1;
    }

    // Assume for now that there this function is not called twice
    assert(state == UNINITIALIZED);

    id = make_request("init_consumer");
    if (id < 0) {
        dbg_printf(DBG, "Error getting the transactional ID!\n");
        return -1;
    }
    dbg_printf(DBG, "Consumer id from server: %d\n", id);
    state = INITIALIZED;
    return id;
}

int ConsumerClient::subscribe(const std::string topic) {
    std::string request = "subscribe," + topic;
    return make_request(request);
}
