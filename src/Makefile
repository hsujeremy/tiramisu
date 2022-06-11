CC = g++ -std=c++14
override CFLAGS += -Wall -W -Wshadow -Wno-format -Wno-unused-parameter

all: server run_producer

server: server.o
	$(CC) $(CFLAGS) server.o -o server

server.o: server.cc
	$(CC) $(CFLAGS) -c server.cc

run_producer: run_producer.o producer.o
	$(CC) $(CFLAGS) run_producer.o producer.o -o run_producer

run_producer.o: run_producer.cc
	$(CC) $(CFLAGS) -c run_producer.cc

producer.o: producer.cc producer.h
	$(CC) $(CFLAGS) -c producer.cc

clean:
	rm -f *.o server run_producer
