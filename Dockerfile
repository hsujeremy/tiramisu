FROM ubuntu:18.04
FROM gcc:latest

WORKDIR /tiramisu

RUN bash -c 'apt-get update && apt-get install -y \
    apt-utils \
    build-essential \
    gcc \
    psmisc \
    python3 \
    python3-pip \
    tmux \
    valgrind \
    strace'

CMD /bin/bash && tmux
