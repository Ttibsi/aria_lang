CFLAGS := -Wall -Wextra -g

%.o: %.c
	gcc $< -c -MMD -MP -o $@ $(CFLAGS)

all: main.o aria.o
	gcc -o aria main.o aria.o

.PHONY: clean
clean:
	rm aria *.d *.o
