#include <iostream>
#include "consumer_client.h"

int main() {
    ConsumerClient* consumer = new ConsumerClient();
    if (consumer->connect_to_server() < 0) {
        perror("Error connecting to server\n");
        return -1;
    }

    consumer->init();
    consumer->subscribe("general");
    consumer->unsubscribe("general");

    delete consumer;
    return 0;
}
