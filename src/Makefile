CC = g++ -std=c++14
override CFLAGS += -Wall -W -Wshadow -Wno-format -Wno-unused-parameter

all: server run_producer_client

server: server.o broker.o
	$(CC) $(CFLAGS) server.o broker.o -o server

server.o: server.cc
	$(CC) $(CFLAGS) -c server.cc

broker.o: broker.cc broker.h
	$(CC) $(CFLAGS) -c broker.cc

run_producer_client: run_producer_client.o producer_client.o
	$(CC) $(CFLAGS) run_producer_client.o producer_client.o -o run_producer_client

run_producer_client.o: run_producer_client.cc
	$(CC) $(CFLAGS) -c run_producer_client.cc

producer_client.o: producer_client.cc producer_client.h
	$(CC) $(CFLAGS) -c producer_client.cc

clean:
	rm -f *.o server run_producer_client
