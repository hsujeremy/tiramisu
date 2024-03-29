#ifndef PRODUCER_H
#define PRODUCER_H

#include <string>
#include "common.h"
#include "client.h"

struct ProducerClient : Client {
    int id;

    // ProducerClient::init()
    //     Gets the transactional ID for this producer back from the server and
    //     sets the state of the producer client accordingly.
    //     Returns the transactional ID on success and -1 otherwise.
    virtual int init();

    // ProducerClient::begin_transaction()
    //     Marks the beginning of a transaction and thus should be called before
    //     the start of any new transaction.
    //     Returns 0 on success, -1 on server error, and -2 on client error.
    int begin_transaction();

    // ProducerClient::send_record(data, topic)
    //     Sends a record to the server as part of a transaction. Must be called
    //     in between calls to begin_transaction and either commit_transaction()
    //     or abort_transaction().
    //   Returns 0 on success, -1 on server error, and -2 on client error.
    int send_record(const int data, const std::string topic = "general");

    // ProducerClient::abort_transaction()
    //     Aborts the ongoing transaction and any unflushed produce messages.
    //     Returns 0 on success, -1 on server error, and -2 on client error.
    int abort_transaction();

    // ProducerClient::commit_transaction()
    //     Commits the ongoing transaction and writes it to disk on the server.
    //     Returns 0 on success, -1 on server error, and -2 on client error.
    int commit_transaction();

    // ProducerClient::close_producer()
    //     Changes the state back to UNINITIALIZED. Must call
    //     `init()` again before making any further transactions.
    //     Returns 0 on success, -1 on server error, and -2 on client error.
    int close_producer();
};

#endif
