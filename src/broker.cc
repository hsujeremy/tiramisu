#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>
#include "broker.h"

Producer::Producer(const int client_socket, const int producer_id) {
    id = producer_id;
    sock = client_socket;
}

Producer::~Producer() {
    if (table) {
        delete table;
    }
}

RequestedAction BrokerManager::parse_request(const std::string request) {
    printf("client request: %s\n", request.c_str());
    // Parse the string and return the request
    if (request.compare("init_transactions") == 0) {
        return INIT_TRANSACTIONS;
    } else if (request.compare("begin_transaction") == 0) {
        return BEGIN_TRANSACTION;
    } else if (request.compare(0, 11, "send_record") == 0) {
        return SEND_RECORD;
    } else if (request.compare("abort_transaction") == 0) {
        return ABORT_TRANSACTION;
    } else if (request.compare("commit_transaction") == 0) {
        return COMMIT_TRANSACTION;
    }
    return UNKNOWN_ACTION;
}

int Producer::init_transactions() {
    return id;
}

int Producer::begin_transaction() {
    table = new Table();
    if (!table) {
        printf("Failed to allocate space for table!\n");
        return -1;
    }
    streaming = true;
    return 0;
}

int Producer::send_record(std::string serialized_args,
                          std::unordered_map<std::string, Table*>& table_map) {
    // Split serialized request into arguments
    std::vector<std::string> substrings;
    std::stringstream sstream(serialized_args);
    while (sstream.good()) {
        std::string substring;
        std::getline(sstream, substring, ',');
        substrings.push_back(substring);
    }

    // Convert data and event_time parameters back to their original types
    assert(substrings.size() == 4 && substrings[0].compare("send_record") == 0);
    std::string topic = substrings[1];
    int data = std::stoi(substrings[2]);
    std::time_t event_time = std::stol(substrings[3]);

    if (!table_map.count(topic)) {
        Table* new_table = new Table();
        // TODO: Need to clean up table_map in BrokerManager destructor
        table_map.insert(std::make_pair(topic, new_table));
    }
    Table* ttable = table_map.at(topic);
    assert(ttable);
    ttable->insert_row(data, event_time);
    return 0;
}

int Producer::cleanup_transaction(const bool save) {
    assert(table);
    if (save) {
        table->flush_to_disk();
    }
    streaming = false;
    delete table;
    table = nullptr;
    return 0;
}

int Producer::abort_transaction() {
    return cleanup_transaction(false);
}

int Producer::commit_transaction() {
    return cleanup_transaction(true);
}

int BrokerManager::execute(ClientType client_type, const int sd,
                           RequestedAction action,
                           std::string serialized_args) {
    // No need to distinguish between client types now since consumer-side is
    // not implemented
    (void)client_type;

    if (!server->sd_client_map.count(sd)) {
        perror("Socket descriptor not found in map!\n");
        return -1;
    }
    Producer* producer = producers[server->sd_client_map.at(sd)];
    assert(producer);

    int result = 0;
    switch (action) {
        case INIT_TRANSACTIONS:
            result = producer->init_transactions();
            break;

        case BEGIN_TRANSACTION:
            result = producer->begin_transaction();
            break;

        case SEND_RECORD:
            result = producer->send_record(serialized_args, table_map);
            break;

        case ABORT_TRANSACTION:
            result = producer->abort_transaction();
            break;

        case COMMIT_TRANSACTION:
            result = producer->commit_transaction();
            break;

        case UNKNOWN_ACTION:
            result = 0;
            break;
    }
    printf("result from server: %d\n", result);
    return result;
}
