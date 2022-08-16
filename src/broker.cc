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
    if (streaming) {
        perror("Streaming already in progress!\n");
        return -1;
    }
    streaming = true;
    return 0;
}

int Producer::send_record(std::string serialized_args) {
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

    // Add to the correct input table
    if (!input_table_map.count(topic)) {
        Table* new_table = new Table(topic);
        input_table_map.insert(std::make_pair(topic, new_table));
    }
    Table* input_table = input_table_map.at(topic);
    assert(input_table);
    input_table->insert_row(data, event_time);
    return 0;
}

int Producer::cleanup_transaction() {
    for (auto const& topic_table : input_table_map) {
        assert(topic_table.second);
        delete topic_table.second;
    }
    return 0;
}

int Producer::abort_transaction() {
    streaming = false;
    return cleanup_transaction();
}

int Producer::commit_transaction(
    std::unordered_map<std::string, Table*>& table_map) {
    streaming = false;
    for (auto const& topic_table : input_table_map) {
        // Find corresponding result table or create if not found
        std::string topic = topic_table.first;
        Table* input_table = topic_table.second;
        if (!table_map.count(topic)) {
            Table* new_table = new Table(topic);
            table_map.insert(std::make_pair(topic, new_table));
        }
        Table* result_table = table_map.at(topic);
        assert(input_table && result_table);

        // Then merge all rows from input table into result table
        for (auto const& row : input_table->rows) {
            result_table->insert_row(row.data, row.event_time);
        }
    }

    // Finally, clean up input table map
    return cleanup_transaction();
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
            result = producer->send_record(serialized_args);
            break;

        case ABORT_TRANSACTION:
            result = producer->abort_transaction();
            break;

        case COMMIT_TRANSACTION:
            result = producer->commit_transaction(table_map);
            break;

        case UNKNOWN_ACTION:
            result = 0;
            break;
    }
    printf("result from server: %d\n", result);
    return result;
}
