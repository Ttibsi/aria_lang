CFLAGS := -Wall -Wextra -g

sources := $(wildcard src/*.c)
objects := $(patsubst src/%.c,build/%.o,$(sources))

tests := $(wildcard tests/*.c)
test_objs := $(patsubst tests/%.c,build/%.o,$(tests))

.PHONY: all
all: aria
	./aria -D examples/01_var.ari

debug:
	$(error   VAR is $(sources))

build:
	mkdir build

build/%.o: src/%.c | build
	$(CC) $< -c -MMD -MP -o $@ $(CFLAGS)

build/main_obj.o: main.c
	$(CC) $< -c -MMD -MP -o $@ $(CFLAGS)

aria: $(objects) build/main_obj.o
	$(CC) $^ -o $@

##########################

build/%.o: tests/%.c | build
	$(CC) $< -c -Isrc -MMD -MP -o $@ $(CFLAGS)

.PHONY: test
test: $(test_objs)
	$(CC) $^ -MMD -MP -std=c23 -o $@

.PHONY: clean
clean:
	rm -rf build
	if [ -f aria ]; then rm aria; fi
	if [ -f test ]; then rm test; fi
