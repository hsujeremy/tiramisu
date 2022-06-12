#ifndef BROKER_H
#define BROKER_H

#include <iostream>
#include "storage.h"

#define MAX_PRODUCERS 1

struct Server {
  int server_socket = -1;
  void setup();
  void handle_client(const int client_socket);
};

enum ClientType {
  PRODUCER,
};

enum RequestedAction {
  INIT_TRANSACTIONS,
  BEGIN_TRANSACTION,
  UNKNOWN_ACTION,
};

struct Producer {
  int socket;
  // Every producer has a single transactional ID during the lifetime of its
  // connection (for now)
  int transactional_id;
  Table *table = nullptr;
  bool streaming = false;

  Producer(int client_socket, int id);

  // Producer::init_transactions()
  //   Return the transactional ID.
  int init_transactions();

  // Producer::begin_transaction()
  //   Creates a new record table. Returns 0 on success and -1 otherwise.
  int begin_transaction();
};

struct BrokerManager {
  Server *server;
  Producer *producers[MAX_PRODUCERS] = {nullptr};

  RequestedAction parse_request(const char *request);
  int execute(ClientType client, RequestedAction action);
};

#endif
