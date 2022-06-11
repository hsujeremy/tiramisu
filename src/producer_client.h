#ifndef PRODUCER_H
#define PRODUCER_H

#include "common.h"

enum ProducerState {
  DISCONNECTED,              // Not connected to the server
  UNINITIALIZED,             // Connected but uninitialized
  INITIALIZED,               // Connected and initialized
};

struct ProducerClient {
  int client_socket;
  int transactional_id;
  ProducerState state = DISCONNECTED;

  void connect_to_server();
  void close_connection();

  void init_transactions();

};

#endif
