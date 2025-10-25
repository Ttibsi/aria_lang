CC := clang-19
CFLAGS := -Wall -Wextra -g -MMD -MP -std=c23

ifeq (${SAN},1)
CFLAGS += -fsanitize=address
endif

sources := $(wildcard src/aria/*.c)
objects := $(patsubst src/aria/%.c,build/%.o,$(sources))

build:
	mkdir build

build/%.o: src/aria/%.c | build
	$(CC) $< -c -o $@ $(CFLAGS)

aria: $(objects) | build
	$(AR) rcs build/libaria.a $(objects)

build/main.o: src/main.c
	$(CC) $< -c -o $@ $(CFLAGS)

.PHONY: all
.DEFAULT_GOAL := all
all: aria build/main.o
	$(CC) build/main.o -Lbuild -laria -o aria $(CFLAGS)
	./aria -D examples/02_func_call.ari; echo -e "\nRetcode: $$?"

##########

.PHONY: test
test: $(objects) | build
	$(CC) $(CFLAGS) -Isrc/aria -c tests/test.c -o build/test.o
	$(CC) -Isrc $(CFLAGS) build/test.o $^ -o $@_exe
	@echo -e "" # newline for navigation

.PHONY: clean
clean:
	rm -rf build
	if [ -f aria ]; 	then rm aria; fi
	if [ -f iris.log ];	then rm iris.log; fi
	if [ -f test_exe ];	then rm test_exe; fi
	if [ -f core ]; 	then rm core; fi

-include $(objects:.o=.d)
