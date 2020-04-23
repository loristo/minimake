CC = gcc
CFLAGS = -Wextra -Wall -Werror -std=c99 -pedantic -g3 -I ./src/

all: test

test: test.o
	gcc -o test test.o

test.o: test.c
	gcc -c -o test.o test.c

clean:
	rm test test.o
