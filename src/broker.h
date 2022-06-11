#ifndef BROKER_H
#define BROKER_H

#include <iostream>

enum RequestedAction {
  INIT_TRANSACTIONS,
  UNKNOWN_ACTION,
};

struct Broker {
  RequestedAction parse_request(const std::string request);

  int init_transactions();
  int execute(RequestedAction action);
};

#endif
