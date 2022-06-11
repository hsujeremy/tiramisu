#include "broker.h"

ProducerMetadata::ProducerMetadata(int client_socket, int id) {
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

int BrokerManager::init_transactions() {
  return 1;
}

int BrokerManager::execute(RequestedAction action) {
  int result = 0;
  switch (action) {
    case INIT_TRANSACTIONS:
      result = init_transactions();
      break;

    case UNKNOWN_ACTION:
      result = 0;
      break;
  }
  return result;
}
