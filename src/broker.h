#ifndef BROKER_H
#define BROKER_H

#include <iostream>
#include "storage.h"

#define MAX_PRODUCERS 1

struct Server {
  int server_socket = -1;

  // Server::setup()
  //   Creates a socket for local communication and sets up a server instance to
  //   listen on the socket and accept new connections. Sets the `server_socket`
  //   member on success and returns prematurely otherwise.
  void setup();

  // Server::handle_client(client_socket)
  //   Handles an incoming client connection on `client_socket` and attempts to
  //   create a Producer object for that client. The server blocks on the client
  //   connection processing requests and sending back responses until the
  //   client closes the connection.
  void handle_client(const int client_socket);
};

enum ClientType {
  PRODUCER,
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
  int socket;
  // Every producer has a single transactional ID during the lifetime of its
  // connection (for now)
  int transactional_id;
  Table* table = nullptr;
  bool streaming = false;

  Producer(const int client_socket, const int id);
  ~Producer();

  // Producer::init_transactions()
  //   Return the transactional ID.
  int init_transactions();

  // Producer::begin_transaction()
  //   Creates a new record table. Returns 0 on success and -1 otherwise.
  int begin_transaction();

  // Producer::send_record()
  //   Creates a new row and performs a relational insert into the table.
  int send_record(std::string serialized_args);

  // Producer::abort_transaction()
  //   Frees the table and sets the `streaming` to false without writing to disk
  //   beforehand.
  int abort_transaction();

  // Producer::commit_transaction()
  //   Commits the transaction by writing it out to disk. Frees the table and
  //   sets `streaming` to false.
  int commit_transaction();

private:
  // Producer::cleanup_transaction(save)
  //   Frees the relevant table and sets `streaming` to false. If `save` is set
  //   as true, then this method also writes the table to disk before freeing.
  int cleanup_transaction(const bool save);
};

struct BrokerManager {
  Server* server;
  Producer* producers[MAX_PRODUCERS] = {nullptr};

  // BrokerManager::parse_request(request)
  //   Parses the request message and determines the correct action type.
  RequestedAction parse_request(const std::string request);

  // BrokerManager::execute(client, action)
  //   Executes the specified action for the specified client, passing in
  //   `serialized_args` if necessary.
  int execute(ClientType client, RequestedAction action,
              std::string serialized_args);
};

#endif
