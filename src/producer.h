#ifndef PRODUCER_H
#define PRODUCER_H

#include "common.h"

enum ProducerState {
  DISCONNECTED,              // Not connected to the server
  UNINITIALIZED,             // Connected but uninitialized
  INITIALIZED,               // Connected and initialized
};

struct Producer {
  int client_socket;
  int transactional_id;
  ProducerState state = DISCONNECTED;

  void connect_to_server();
  void close_connection();

  // What does this even do?
  // 1. Sets the transactional_id on the server side
  // 2. Changes the state of the producer
  void init_transactions();
};

#endif
