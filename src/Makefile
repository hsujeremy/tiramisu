CC = g++ -std=c++14
override CFLAGS += -Wall -W -Wshadow -Wno-format -Wno-unused-parameter

all: streamer run_producer

streamer: streamer.o
	$(CC) $(CFLAGS) streamer.o -o streamer

streamer.o: streamer.cc
	$(CC) $(CFLAGS) -c streamer.cc

run_producer: run_producer.o producer.o
	$(CC) $(CFLAGS) run_producer.o producer.o -o run_producer

run_producer.o: run_producer.cc
	$(CC) $(CFLAGS) -c run_producer.cc

producer.o: producer.cc producer.h
	$(CC) $(CFLAGS) -c producer.cc

clean:
	rm -f *.o streamer run_producer
