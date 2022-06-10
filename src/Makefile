CC = g++ -std=c++14
override CFLAGS += -Wall -W -Wshadow -Wno-format -Wno-unused-parameter

streamer: streamer.o
	$(CC) $(CFLAGS) streamer.o -o streamer

streamer.o: streamer.cc
	$(CC) $(CFLAGS) -c streamer.cc

producer: producer.o
	$(CC) $(CFLAGS) producer.o -o producer

producer.o: producer.cc
	$(CC) $(CFLAGS) -c producer.cc

clean:
	rm -f *.o streamer producer
