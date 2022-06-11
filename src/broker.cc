#include "broker.h"

ProducerMetadata::ProducerMetadata(int client_socket) {
  socket = client_socket;
}

RequestedAction Broker::parse_request(const std::string request) {
   // Parse the string and return the request
  if (request.compare("init_transactions") == 0) {
    return INIT_TRANSACTIONS;
  }
  return UNKNOWN_ACTION;
}

int Broker::init_transactions() {
  return 1;
}

int Broker::execute(RequestedAction action) {
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
