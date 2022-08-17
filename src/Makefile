CC = g++ -std=c++14
override CFLAGS += -Wall -W -Wshadow -Wno-format -Wno-unused-parameter

all: server run_producer_client

server: server.o broker.o storage.o common.o
	$(CC) $(CFLAGS) server.o broker.o storage.o common.o -o server

server.o: server.cc
	$(CC) $(CFLAGS) -c server.cc

broker.o: broker.cc broker.h
	$(CC) $(CFLAGS) -c broker.cc

storage.o: storage.cc storage.h
	$(CC) $(CFLAGS) -c storage.cc

run_producer_client: run_producer_client.o producer_client.o common.o
	$(CC) $(CFLAGS) run_producer_client.o producer_client.o common.o -o run_producer_client

run_producer_client.o: run_producer_client.cc
	$(CC) $(CFLAGS) -c run_producer_client.cc

producer_client.o: producer_client.cc producer_client.h
	$(CC) $(CFLAGS) -c producer_client.cc

common.o: common.cc common.h
	$(CC) $(CFLAGS) -c common.cc

clean:
	rm -f *.o server run_producer_client
