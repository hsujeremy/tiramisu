#ifndef PRODUCER_H
#define PRODUCER_H

#include <string>
#include "common.h"

enum ProducerState {
  DISCONNECTED,       // Not connected to the server
  UNINITIALIZED,      // Connected but uninitialized
  INITIALIZED,        // Connected and initialized
};

struct ProducerClient {
  int client_socket;
  int transactional_id;
  ProducerState state = DISCONNECTED;

  // ProducerClient::connect_to_server()
  //   Attempts to connect to the server. Sets the `client_socket` member on
  //   success and returns prematurely otherwise. Sets the producer state to
  //   UNITIALIZED.
  void connect_to_server();

  // ProducerClient::make_request(request, response)
  //   Make request to server and save server output in `response`. Return 0 on
  //   success and -1 otherwise.
  int make_request(const std::string request, std::string* response);

  // ProducerClient::close_connection()
  //   Closes the connection the server.
  void close_connection();

  // ProducerClient::init_transactions()
  //   Gets the transactional ID for this producer back from the server and
  //   sets the state of the producer client accordingly. Returns the
  //   transactional ID on success and -1 otherwise.
  int init_transactions();

  // ProducerClient::begin_transaction()
  //   Marks the beginning of a transaction and thus should be called before the
  //   start of any new transaction.
  void begin_transaction();

  // ProducerClient::send_record(data)
  //   Sends a record to the server as part of a transaction. Must be called
  //   in between calls to begin_transaction and either commit_transaction() or
  //   abort_transaction().
  void send_record(const int data);

  // ProducerClient::abort_transaction()
  //   Aborts the ongoing transaction and any unflushed produce messages.
  void abort_transaction();

  // ProducerClient::commit_transaction()
  //   Commits the ongoing transaction and writes it to disk on the server.
  void commit_transaction();

  // ProducerClient::close_producer()
  //   Changes the state back to UNINITIALIZED. Must call `init_transactions()`
  //   again before making any further transactions.
  void close_producer();
};

#endif
