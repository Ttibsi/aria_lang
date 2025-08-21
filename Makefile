CFLAGS := -Wall -Wextra -g

sources := $(filter-out src/test.c,$(wildcard src/*.c))
objects := $(patsubst src/%.c,build/%.o,$(sources))

.PHONY: all
all: aria

debug:
	$(error   VAR is $(sources))

build:
	mkdir build

build/%.o: src/%.c | build
	$(CC) $< -c -MMD -MP -o $@ $(CFLAGS)

build/main_obj.o: main.c
	$(CC) $< -c -MMD -MP -o $@ $(CFLAGS)

.DEFAULT_GOAL := aria
aria: $(objects) build/main_obj.o
	$(CC) $^ -o $@ $(san)

.PHONY: test
test:
	echo "todo"

.PHONY: clean
clean:
	rm -rf build
	rm aria
