#define _XOPEN_SOURCE

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "common.h"
#include "producer_client.h"

int main() {
  // // Tbh we can just make the transactions here for now
  // ProducerClient* prod = new ProducerClient;
  // prod->connect_to_server();
  // if (prod->client_socket < 0) {
  //   exit(1);
  // }

  // prod->init_transactions();
  // prod->begin_transaction();
  // for (size_t i = 0; i < 10; ++i) {
  //   prod->send_record(i);
  // }
  // prod->commit_transaction();
  // prod->close_producer();
  // prod->close_connection();

  // delete prod;
  // return 0;

  ProducerClient* prod = new ProducerClient();
  int r = prod->connect_to_server();
  if (r < 0) {
    perror("Error connecting to server!\n");
    return -1;
  }

  char buf[1024] = {0};
  std::string message = "init_transactions";

  send(prod->client_socket, message.c_str(), message.length(), 0);
  read(prod->client_socket, buf, 1024);
  printf("From server: %s\n", buf);

  close(prod->client_fd);
  return 0;

}
