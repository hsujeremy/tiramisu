# Tiramisu

Tiramisu is a lightweight event streaming platform designed for producers to
easily stream records.

Why is this system called Tiramisu? I guess it sounds cool and is one of my
favorite desserts. And it also rhymes with my name!

## Docker Setup

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

## Running Tiramisu

You can compile everything by running `make all`. First start the server in one
shell with `./server`. You can then run the provided producer client test
program `./run_producer_client`, or create and run your own!
