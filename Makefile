CC = gcc
CFLAGS = -Wextra -Wall -Werror -std=c99 -pedantic
SRCDIR = ./src
OBJ = minimake.o

all: minimake

minimake: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -c

clean:
	$(RM) minimake $(OBJ)
