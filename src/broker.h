#ifndef BROKER_H
#define BROKER_H

#include <iostream>

#define MAX_PRODUCERS 1

enum RequestedAction {
  INIT_TRANSACTIONS,
  UNKNOWN_ACTION,
};

struct Server {
  int server_socket = -1;

  void setup();
  void handle_client(const int client_socket);
};

struct Producer {
  int socket;
  // Every producer has a single transactional ID during the lifetime of its
  // connection (for now)
  int transactional_id;

  Producer(int client_socket, int id);
};

struct BrokerManager {
  Server *server;
  Producer *producers[MAX_PRODUCERS] = {nullptr};

  RequestedAction parse_request(const std::string request);

  int init_transactions();
  int execute(RequestedAction action);
};

#endif
