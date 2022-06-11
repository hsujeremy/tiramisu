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
  ProducerState state = DISCONNECTED;

  void connect_to_server();
  void close_connection();
};

#endif
