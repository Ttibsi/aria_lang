CFLAGS := -Wall -Wextra -g

*.o: *.c
	gcc $< -c -MMD -MP -o $@ $(CFLAGS)

main: main.o idk.o
	gcc -o main main.o idk.o

