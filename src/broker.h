#ifndef BROKER_H
#define BROKER_H

#include <iostream>

#define MAX_PRODUCERS 1

enum RequestedAction {
  INIT_TRANSACTIONS,
  UNKNOWN_ACTION,
};

struct ProducerMetadata {
  int socket;
  int transactional_id;

  ProducerMetadata(int client_socket);
};

struct Broker {
  ProducerMetadata *producers[MAX_PRODUCERS] = {nullptr};

  RequestedAction parse_request(const std::string request);

  int init_transactions();
  int execute(RequestedAction action);
};

#endif
