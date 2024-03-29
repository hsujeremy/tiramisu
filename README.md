# Tiramisu

Tiramisu is a lightweight event streaming engine that supports concurrent
clients and ACID transactions while providing simple external API.

Why is this system called Tiramisu? I guess it sounds cool and is one of my
favorite desserts. And it also rhymes with my name!

## Design

Similar to other streaming engines, Tiramisu serves a intermediate platform (known as a message broker) between producers that are sending streams of data and consumers that are reading and processing it.

A producer sends over data interpreted by the system as transactions, where each transaction's start and end are set with explicit API calls. A transaction may contain one or more records.

Data is stored in tables organized per-topic, where each topic is a string describing what the data refers to (e.g. "houses", "cars", "trips", etc.). Note that a table may contain records pushed from any number of producers. Furthermore, the Tiramisu broker stores only fully-completed transactions in its result tables, and only persists these result tables if the server shuts down. Before a transaction is marked as committed, intermediate records are stored in a per-producer table, also indexed by topic. 

### Handling Multiple Clients

Unlike other implementations of servers that can scale to process multiple
clients, Tiramisu **does not** assign each process its own thread. Instead,
Tiramisu makes use of Linux's `select()` system call, which allows the server to
keep track of multiple socket descriptors simultaneously.

## How to Run

### Docker Setup

Tiramisu is written in C++14 in a Linux environment. The following `make`
commands can help you easily setup the Docker environment all required/helpful
dependencies and tools:
```bash
$ # Build the image
$ make build
$ # Start a container
$ make startcontainer
$ # Run the container
$ make runcontainer
$ # Stop the container
$ make stopcontainer
```

### Compiling and Running the Server/Producer Programs

You can compile everything by running `make all`. First start the server in one
shell with `./server`. You can then run the provided producer client test
program `./run_producer_client`, or create and run your own!
