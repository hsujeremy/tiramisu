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

  // ProducerClient::make_request(request, response)
  //   Make request to server and save server output in `response`. Return 0 on
  //   success and -1 otherwise.
  int make_request(const std::string request, std::string *response);

  void close_connection();

  // ProducerClient::init_transactions()
  //   Gets the transactional ID for this producer back from the server and
  //   sets the state of the producer client accordingly. Returns the
  //   transactional ID on success and -1 otherwise.
  int init_transactions();

  // ProducerClient::close_producer()
  //   Changes the state back to UNINITIALIZED. Must call `init_transactions()`
  //   again before making any further transactions.
  void close_producer();
};

#endif
