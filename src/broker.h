#ifndef BROKER_H
#define BROKER_H

#include <iostream>
#include <unordered_map>
#include "storage.h"

#define MAX_PRODUCERS 15
#define MAX_CONSUMERS 15

enum ClientType {
    PRODUCER,
    CONSUMER,
};

struct Server {
    int server_socket = -1;
    std::unordered_map<size_t, size_t> sd_client_map;
};

enum RequestedAction {
    INIT_TRANSACTIONS,
    BEGIN_TRANSACTION,
    SEND_RECORD,
    ABORT_TRANSACTION,
    COMMIT_TRANSACTION,
    UNKNOWN_ACTION,
};

struct Producer {
    int id;
    int sock;
    Table* table = nullptr;
    bool streaming = false;
    std::vector<int> subscribers;

    Producer(const int client_socket, const int producer_id);
    ~Producer();

    // Producer::init_transactions()
    //     Return the transactional ID.
    int init_transactions();

    // Producer::begin_transaction()
    //     Creates a new record table. Returns 0 on success and -1 otherwise.
    int begin_transaction();

    // Producer::send_record()
    //     Creates a new row and performs a relational insert into the table.
    int send_record(std::string serialized_args,
                    std::unordered_map<std::string, Table*>& table_map);

    // Producer::abort_transaction()
    //     Frees the table and sets the `streaming` to false without writing to
    //     disk beforehand.
    int abort_transaction();

    // Producer::commit_transaction()
    //     Commits the transaction by writing it out to disk. Frees the table
    //     and sets `streaming` to false.
    int commit_transaction();

private:
    // Producer::cleanup_transaction(save)
    //     Frees the relevant table and sets `streaming` to false. If `save` is
    //     set as true, then this method also writes the table to disk before
    //     freeing.
    int cleanup_transaction(const bool save);
};

struct Consumer {
    int id;

    Consumer(const int consumer_id);
    void subscribe();
    void unsubscribe();
};

struct BrokerManager {
    Server* server;
    Producer* producers[MAX_PRODUCERS] = {nullptr};
    Consumer* consumers[MAX_CONSUMERS] = {nullptr};
    std::unordered_map<std::string, Table*> table_map;

    // BrokerManager::parse_request(request)
    //     Parses the request message and returns the correct action type.
    RequestedAction parse_request(const std::string request);

    // BrokerManager::execute(client, action)
    //     Executes the specified action for the specified client, passing in
    //     `serialized_args` if necessary.
    //     Returns a nonnegative integer on success and -1 otherwise.
    int execute(ClientType client, const int sd, RequestedAction action,
                std::string serialized_args);
};

#endif
