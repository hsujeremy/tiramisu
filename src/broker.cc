#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>
#include "broker.h"

Producer::Producer(const int client_socket, const int id) {
  socket = client_socket;
  transactional_id = id;
}

Producer::~Producer() {
  if (table) {
    delete table;
  }
}

RequestedAction BrokerManager::parse_request(const std::string request) {
  printf("client request: %s\n", request.c_str());
  // Parse the string and return the request
  if (request.compare("init_transactions") == 0) {
    return INIT_TRANSACTIONS;
  } else if (request.compare("begin_transaction") == 0) {
    return BEGIN_TRANSACTION;
  } else if (request.compare(0, 11, "send_record") == 0) {
    return SEND_RECORD;
  } else if (request.compare("abort_transaction") == 0) {
    return ABORT_TRANSACTION;
  } else if (request.compare("commit_transaction") == 0) {
    return COMMIT_TRANSACTION;
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

int Producer::send_record(std::string serialized_args) {
  // Split serialized request into arguments
  std::vector<std::string> substrings;
  std::stringstream sstream(serialized_args);
  while (sstream.good()) {
    std::string substring;
    std::getline(sstream, substring, ',');
    substrings.push_back(substring);
  }

  // Convert data and event_time parameters back to their original types
  assert(substrings.size() == 3 && substrings[0].compare("send_record") == 0);
  int data = std::stoi(substrings[1]);
  std::time_t event_time = std::stol(substrings[2]);

  assert(table);
  table->insert_row(data, event_time);
  return 0;
}

int Producer::cleanup_transaction(const bool save) {
  assert(table);
  if (save) {
    table->flush_to_disk();
  }
  streaming = false;
  delete table;
  table = nullptr;
  return 0;
}

int Producer::abort_transaction() {
  return cleanup_transaction(false);
}

int Producer::commit_transaction() {
  return cleanup_transaction(true);
}

int BrokerManager::execute(ClientType client, RequestedAction action,
                           std::string serialized_args) {
  // No need to distinguish between client types now since consumer-side is not
  // implemented
  (void)client;

  Producer* producer;
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

    case SEND_RECORD:
      result = producer->send_record(serialized_args);
      break;

    case ABORT_TRANSACTION:
      result = producer->abort_transaction();
      break;

    case COMMIT_TRANSACTION:
      result = producer->commit_transaction();
      break;

    case UNKNOWN_ACTION:
      result = 0;
      break;
  }
  printf("result from server: %d\n", result);
  return result;
}
