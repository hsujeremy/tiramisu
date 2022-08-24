CC = g++ -std=c++14
override CFLAGS += -Wall -W -Wshadow -Wno-format -Wno-unused-parameter

all: server run_producer_client run_consumer_client

####### Automatic dependency magic #######
DEPSDIR := .deps
BUILDSTAMP := $(DEPSDIR)/rebuildstamp
DEPFILES := $(wildcard $(DEPSDIR)/*.d)
ifneq ($(DEPFILES),)
include $(DEPFILES)
endif
DEPCFLAGS = -MD -MF $(DEPSDIR)/$*.d -MP -DSOCK_PATH=$(SOCK_PATH)

# Dependency compilation
ifneq ($(DEP_CC),$(CC) $(CFLAGS) $(DEPCFLAGS) $(O))
DEP_CC := $(shell mkdir -p $(DEPSDIR); echo >$(BUILDSTAMP); echo "DEP_CC:=$(CC) $(CFLAGS) $(DEPCFLAGS) $(O)" >$(DEPSDIR)/_cc.d)
endif

# Make sure dependency directories are generated
$(DEPSDIR)/stamp $(BUILDSTAMP):
	mkdir -p $(@D)
	touch $@

####### Targets #######

server: server.o broker.o storage.o common.o
	$(CC) $(CFLAGS) $(DEPCFLAGS) -o $@ $^

run_producer_client: run_producer_client.o client.o producer_client.o common.o
	$(CC) $(CFLAGS) $(DEPCFLAGS) -o $@ $^

run_consumer_client: run_consumer_client.o client.o consumer_client.o common.o
	$(CC) $(CFLAGS) $(DEPCFLAGS) -o $@ $^

clean:
	rm -f *.o server run_producer_client run_consumer_client
