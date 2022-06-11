#ifndef PRODUCER_H
#define PRODUCER_H

#include "common.h"

struct Producer {
  int client_socket;

  void connect_to_server();
  void close_connection();
};

#endif
