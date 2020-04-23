CC = gcc
CFLAGS = -Wextra -Wall -Werror -std=c99 -pedantic -g3 -I ./src/
SRCDIR = ./src
OBJ = $(SRCDIR)/minimake.o $(SRCDIR)/options/options.o $(SRCDIR)/parse/linked.o $(SRCDIR)/parse/parse.o $(SRCDIR)/rule/rule.o $(SRCDIR)/variable/variable.o

all: minimake

minimake: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

check: CFLAGS += -fsanitize=address
check: clean all
	@./tests/test8.py ./minimake ./tests/

clean:
	$(RM) minimake $(OBJ)

.PHONY: all clean check
