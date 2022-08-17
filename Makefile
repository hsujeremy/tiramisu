BASE_DIR := $(shell pwd)
DOCKER_CMD := docker

build:
	$(DOCKER_CMD) build -t tiramisu .

startcontainer:
	$(eval DOCKER_CONT_ID := $(shell $(DOCKER_CMD) container run \
		-v $(BASE_DIR):/tiramisu \
		-d --rm -t --privileged -i tiramisu bash))
	echo $(DOCKER_CONT_ID) > status.current_container_id

runcontainer:
	$(eval DOCKER_CONT_ID := $(shell cat status.current_container_id | awk '{print $1}'))
	$(DOCKER_CMD) exec -it $(DOCKER_CONT_ID) bash

stopcontainer:
	$(eval DOCKER_CONT_ID := $(shell cat status.current_container_id | awk '{print $1}'))
	$(DOCKER_CMD) stop $(DOCKER_CONT_ID)
	rm status.current_container_id
