#define _XOPEN_SOURCE

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "common.h"
#include "producer_client.h"

#include <arpa/inet.h>

#define PORT 8888

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

  struct sockaddr_in server_addr;
  char buf[1024] = {0};
  std::string message = "init_transactions";

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Error creating socket\n");
    return -1;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  // Convert IPv4 and IPv6 address from text to binary
  int r = inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
  if (r <= 0) {
    perror("Invalid address! Address not supported\n");
    return -1;
  }

  int client_fd = connect(sock, (struct sockaddr*)&server_addr,
                          sizeof(struct sockaddr));
  if (client_fd < 0) {
    perror("Connection failed\n");
    return -1;
  }

  send(sock, message.c_str(), strlen(message.c_str()), 0);
  ssize_t valread = read(sock, buf, 1024);
  printf("From server: %s\n", buf);

  close(client_fd);
  return 0;

}
