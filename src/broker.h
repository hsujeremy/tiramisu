#ifndef BROKER_H
#define BROKER_H

#include <ctime>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "common.h"
#include "storage.h"

#define BUFSIZE 1024
#define MAX_PENDING 3
#define MAX_PRODUCERS 15
#define MAX_CONSUMERS 15
#define MAX_CLIENTS 30

struct BrokerManager;

enum ClientType {
    PRODUCER,
    CONSUMER,
    UNSPECIFIED,
};

struct ClientMetadata {
    bool filled = false;
    int sock = 0;
    int idx = -1;
    ClientType type = UNSPECIFIED;
};

struct Server {
    BrokerManager* broker = nullptr;
    int server_socket = -1;
    int client_sockets[MAX_CLIENTS] = {0};
    std::unordered_map<size_t, ClientMetadata> sockfd2client;

    ssize_t send_message(int socket, std::string payload);

    // recv_message(socket, payload)
    //     Returns the number of bytes read and updates the payload.
    ssize_t recv_message(int socket, std::string* payload);

    void cleanup_client(const int idx);
};

enum RequestedAction {
    INIT_PRODUCER,
    INIT_CONSUMER,
    BEGIN_TRANSACTION,
    SEND_RECORD,
    ABORT_TRANSACTION,
    COMMIT_TRANSACTION,
    SUBSCRIBE,
    UNSUBSCRIBE,
    UNKNOWN_ACTION,
};

struct TableMap {
    std::unordered_map<std::string, Table*> map;

    Table* find_or_create_table(const std::string topic);
    void flush_tables();
    void free_tables();
};

struct Producer {
    int id;
    int sock;
    TableMap input_tables;
    bool streaming = false;
    std::vector<int> subscribers;

    Producer(const int producer_sock, const int producer_id);

    // Producer::init_transactions()
    //     Return the transactional ID.
    int init_transactions();

    // Producer::begin_transaction()
    //     Creates a new record table. Returns 0 on success and -1 otherwise.
    int begin_transaction();

    // Producer::send_record(serialized_args)
    //     Creates a new row and performs a relational insert into the table.
    int send_record(std::string serialized_args);

    // Producer::abort_transaction()
    //     Frees the table and sets the `streaming` to false without writing to
    //     disk beforehand.
    int abort_transaction();

    // Producer::commit_transaction(result_tables)
    //     Commits the transaction by writing it out to disk. Frees the table
    //     and sets `streaming` to false.
    int commit_transaction(TableMap& result_tables);

private:
    // Producer::cleanup_transaction(save)
    //     Frees the relevant table and sets `streaming` to false. If `save` is
    //     set as true, then this method also writes the table to disk before
    //     freeing.
    int cleanup_transaction();
};

struct Consumer {
    int id;
    int sock;
    std::time_t ts_offset;
    std::unordered_set<std::string> subscriptions;

    Consumer(const int consumer_sock, const int consumer_id);
    int subscribe(TableMap& result_tables, std::string serialized_args);
    int unsubscribe(std::string serialized_args);
};

struct BrokerManager {
    Server* server;
    Producer* producers[MAX_PRODUCERS] = {nullptr};
    Consumer* consumers[MAX_CONSUMERS] = {nullptr};
    TableMap result_tables;

    ~BrokerManager();

    // BrokerManager::parse_request(request)
    //     Parses the request message and returns the correct action type.
    RequestedAction parse_request(const std::string request);

    int init_producer(const int sd);
    int init_consumer(const int sd);

    // BrokerManager::execute(sd, action, serialized_args)
    //     Executes the specified action for the specified client, passing in
    //     `serialized_args` if necessary.
    //     Returns a nonnegative integer on success and -1 otherwise.
    int execute(const int sd, RequestedAction action,
                std::string serialized_args);

    void deallocate_client(const ClientMetadata metadata);

private:
    void deallocate_producer(const int idx);
    void deallocate_consumer(const int idx);
};

#endif
