#define _XOPEN_SOURCE

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "common.h"
#include "producer_client.h"

int main() {
  ProducerClient* prod = new ProducerClient();
  int r = prod->connect_to_server();
  if (r < 0) {
    perror("Error connecting to server!\n");
    return -1;
  }

  prod->init_transactions();
  prod->begin_transaction();
  for (size_t i = 0; i < 10; ++i) {
    prod->send_record(i);
  }
  prod->commit_transaction();
  prod->close_producer();

  // TODO: Do we also need to close the socket?
  close(prod->client_socket);

  delete prod;
  return 0;
}
