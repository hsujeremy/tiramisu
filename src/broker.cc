#include <cstring>
#include "broker.h"

Producer::Producer(int client_socket, int id) {
  socket = client_socket;
  transactional_id = id;
}

RequestedAction BrokerManager::parse_request(const char *request) {
  // Parse the string and return the request
  if (strcmp(request, "init_transactions") == 0) {
    return INIT_TRANSACTIONS;
  } if (strcmp(request, "begin_transaction") == 0) {
    return BEGIN_TRANSACTION;
  }
  return UNKNOWN_ACTION;
}

int Producer::init_transactions() {
  return transactional_id;
}

int Producer::begin_transaction() {
  table = new Table();
  if (!table) {
    printf("Failed to allocate space for table!\n");
    return -1;
  }
  streaming = true;
  return 0;
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

    case BEGIN_TRANSACTION:
      result = producer->begin_transaction();
      break;

    case UNKNOWN_ACTION:
      result = 0;
      break;
  }
  printf("result %d\n", result);
  return result;
}
