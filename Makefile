CC := clang
CFLAGS := -Wall -Wextra -g -MMD -MP

sources := $(wildcard src/*.c)
objects := $(patsubst src/%.c,build/%.o,$(sources))

.PHONY: all
all: aria
	./aria -D examples/01_var.ari

debug:
	$(error   VAR is $(sources))

build:
	mkdir build

build/%.o: src/%.c | build
	$(CC) $< -c -o $@ $(CFLAGS)

build/main_obj.o: main.c
	$(CC) $< -c -o $@ $(CFLAGS)

aria: $(objects) build/main_obj.o
	$(CC) $^ -o $@

##########################

.PHONY: test
test: $(objects) | build
	$(CC) $(CFLAGS) -std=c23 -Isrc -c tests/test.c -o build/test.o
	$(CC) -Isrc build/test.o $^ -o $@

.PHONY: clean
clean:
	rm -rf build
	if [ -f aria ]; then rm aria; fi
	if [ -f test ]; then rm test; fi

-include $(objects:.o=.d)
