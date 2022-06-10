#ifndef COMMON_H
#define COMMON_H

#ifndef SOCK_PATH
#define SOCK_PATH "producer_unix_socket"
#endif

enum Status {
  OK_DONE,
  UNCATEGORIZED,
};

struct Packet {
  Status status;
  unsigned length;
  char *payload;
};

#endif
