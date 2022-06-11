#include "broker.h"

Producer::Producer(int client_socket, int id) {
  socket = client_socket;
  transactional_id = id;
}

RequestedAction BrokerManager::parse_request(const std::string request) {
   // Parse the string and return the request
  if (request.compare("init_transactions") == 0) {
    return INIT_TRANSACTIONS;
  }
  return UNKNOWN_ACTION;
}

int Producer::init_transactions() {
  return transactional_id;
}

int BrokerManager::execute(ClientType client, RequestedAction action) {
  // No need to distinguish between client types now since consumer-side is not
  // implemented
  (void)client;

  Producer *producer;
  // Trivial for now since there is at most one producer but good futureproofing
  for (int i = 0; i < MAX_PRODUCERS; ++i) {
    producer = producers[i];
  }

  int result = 0;
  switch (action) {
    case INIT_TRANSACTIONS:
      result = producer->init_transactions();
      break;

    case UNKNOWN_ACTION:
      result = 0;
      break;
  }
  return result;
}
