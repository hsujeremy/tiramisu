#define _XOPEN_SOURCE

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "common.h"
#include "producer_client.h"

#define STDIN_BUFSIZE 1024

int main() {
  // Tbh we can just make the transactions here for now
  ProducerClient *prod = new ProducerClient;
  prod->connect_to_server();
  if (prod->client_socket < 0) {
    exit(1);
  }

  prod->init_transactions();
  prod->begin_transaction();
  prod->send_record(1);
  prod->commit_transaction();
  prod->close_producer();
  prod->close_connection();

  delete prod;
  return 0;
}
