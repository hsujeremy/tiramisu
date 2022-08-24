#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>
#include "broker.h"
#include "common.h"

// -----------------------------------------------------------------------------
// TableMap implementation

Table* TableMap::find_or_create_table(const std::string topic) {
    if (!map.count(topic)) {
        Table* new_table = new Table(topic);
        if (!new_table) {
            perror("Unable to create new table\n");
            return nullptr;
        }
        map.insert(std::make_pair(topic, new_table));
    }
    return map.at(topic);
}

void TableMap::flush_tables() {
    for (auto const& topic_table : map) {
        assert(topic_table.second);
        topic_table.second->flush_to_disk();
    }
}

void TableMap::free_tables() {
    for (auto const& topic_table : map) {
        assert(topic_table.second);
        delete topic_table.second;
    }
}

// -----------------------------------------------------------------------------
// Producer implementation

Producer::Producer(const int client_socket, const int producer_id) {
    id = producer_id;
    sock = client_socket;
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
    Table* input_table = input_tables.find_or_create_table(topic);
    assert(input_table);
    input_table->insert_row(data, event_time);
    return 0;
}

int Producer::cleanup_transaction() {
    input_tables.free_tables();
    return 0;
}

int Producer::abort_transaction() {
    streaming = false;
    return cleanup_transaction();
}

int Producer::commit_transaction(TableMap& result_tables) {
    streaming = false;
    for (auto const& topic_table : input_tables.map) {
        // Find corresponding result table or create if not found
        std::string topic = topic_table.first;
        Table* input_table = topic_table.second;
        Table* result_table = result_tables.find_or_create_table(topic);
        assert(input_table && result_table);

        // Then merge all rows from input table into result table
        for (auto const& row : input_table->rows) {
            result_table->insert_row(row.data, row.event_time);
        }
    }

    // Finally, clean up input table map
    return cleanup_transaction();
}

// -----------------------------------------------------------------------------
// Consumer implementation

Consumer::Consumer(const int consumer_sock, const int consumer_id) {
    id = consumer_id;
    sock = consumer_sock;
    ts_offset = std::time(nullptr);
}

int Consumer::subscribe(const std::string topic) {
    subscriptions.insert(topic);
    return 0;
}

// -----------------------------------------------------------------------------
// BrokerManager implementation

RequestedAction BrokerManager::parse_request(const std::string request) {
    dbg_printf(DBG, "client request: %s\n", request.c_str());
    // Parse the string and return the request
    if (request.compare("init_producer") == 0) {
        return INIT_PRODUCER;
    } else if (request.compare("init_consumer") == 0) {
        return INIT_CONSUMER;
    } else if (request.compare("begin_transaction") == 0) {
        return BEGIN_TRANSACTION;
    } else if (request.compare(0, 11, "send_record") == 0) {
        return SEND_RECORD;
    } else if (request.compare("abort_transaction") == 0) {
        return ABORT_TRANSACTION;
    } else if (request.compare("commit_transaction") == 0) {
        return COMMIT_TRANSACTION;
    } else if (request.compare(0, 9, "subscribe") == 0) {
        return SUBSCRIBE;
    }
    return UNKNOWN_ACTION;
}

int BrokerManager::init_producer(const int sd) {
    int idx = -1;
    for (int i = 0; i < MAX_PRODUCERS; ++i) {
        if (!producers[i]) {
            idx = i;
            producers[i] = new Producer(sd, idx);
            ClientMetadata* metadata = &server->sd_client_map.at(sd);
            metadata->idx = i;
            metadata->type = PRODUCER;
            dbg_printf(DBG, "Created producer with id %d\n", idx);
            break;
        }
    }
    return idx;
}

int BrokerManager::init_consumer(const int sd) {
    int idx = -1;
    for (int i = 0; i < MAX_CONSUMERS; ++i) {
        if (!consumers[i]) {
            idx = i;
            consumers[i] = new Consumer(sd, idx);
            ClientMetadata* metadata = &server->sd_client_map.at(sd);
            metadata->idx = i;
            metadata->type = CONSUMER;
            dbg_printf(DBG, "Created consumer with id %d\n", idx);
            break;
        }
    }
    return idx;
}

int BrokerManager::execute(const int sd, RequestedAction action,
                           std::string serialized_args) {
    if (action == INIT_PRODUCER) {
        return init_producer(sd);
    } else if (action == INIT_CONSUMER) {
        return init_consumer(sd);
    }

    if (!server->sd_client_map.count(sd)) {
        perror("Socket descriptor not found in map!\n");
        return -1;
    }
    ClientMetadata metadata = server->sd_client_map.at(sd);
    // TODO: Distinguish between client types
    if (metadata.type == PRODUCER) {
        Producer* producer = producers[metadata.idx];
        assert(producer);

        int result = 0;
        switch (action) {
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
                result = producer->commit_transaction(result_tables);
                break;

            case UNKNOWN_ACTION:
                result = 0;
                break;
        }
        dbg_printf(DBG, "result from server: %d\n", result);
        return result;
    } else {
        return 0;
    }
}

void BrokerManager::deallocate_producer(const int idx) {
    Producer* exited = producers[idx];
    producers[idx] = nullptr;
    delete exited;
}

void BrokerManager::deallocate_consumer(const int idx) {
    Consumer* exited = consumers[idx];
    consumers[idx] = nullptr;
    delete exited;
}

void BrokerManager::deallocate_client(const ClientMetadata metadata) {
    switch (metadata.type) {
        case PRODUCER:
            deallocate_producer(metadata.idx);
            break;

        case CONSUMER:
            deallocate_consumer(metadata.idx);
            break;

        default:
            break;
    }
}

BrokerManager::~BrokerManager() {
    result_tables.flush_tables();
    result_tables.free_tables();
}

// -----------------------------------------------------------------------------
// Server implementation

void Server::cleanup_client(const int idx) {
    const int sd = client_sockets[idx];
    if (!sd_client_map.count(sd)) {
        dbg_printf(DBG, "Socket not found in map\n");
        return;
    }
    ClientMetadata metadata = sd_client_map.at(sd);
    broker->deallocate_client(metadata);
    sd_client_map.erase(sd);
    client_sockets[idx] = 0;
}
