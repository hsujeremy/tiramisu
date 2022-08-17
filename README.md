# Tiramisu

Tiramisu is a lightweight event streaming engine that supports concurrent
clients and ACID transactions while providing simple external API.

Why is this system called Tiramisu? I guess it sounds cool and is one of my
favorite desserts. And it also rhymes with my name!

## Design

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
