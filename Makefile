CC := clang
CFLAGS := -Wall -Wextra -g -MMD -MP -std=c23

sources := $(wildcard src/*.c)
objects := $(patsubst src/%.c,build/%.o,$(sources))

.PHONY: all
all: aria
	./aria -D examples/01_var.ari; echo -e "\nRetcode: $$?"

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
	$(CC) $(CFLAGS) -Isrc -c tests/test.c -o build/test.o
	$(CC) -Isrc build/test.o $^ -o $@

.PHONY: clean
clean:
	rm -rf build
	if [ -f aria ]; then rm aria; fi
	if [ -f test ]; then rm test; fi
	if [ -f core ]; then rm core; fi

-include $(objects:.o=.d)
