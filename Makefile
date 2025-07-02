CFLAGS := -Wall -Wextra -g

%.o: %.c
	gcc $< -c -MMD -MP -o $@ $(CFLAGS)

aria: main.o aria.o
	gcc -o main main.o idk.o

