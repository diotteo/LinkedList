src := $(wildcard *.c)
CC = gcc
CFLAGS = -pedantic -ansi -Wall -Wextra

ifneq ($(DEBUG),0)
	CFLAGS += -g
endif


.PHONY: tests


tests: $(src)
	$(CC) -o $@ $(CFLAGS) $^
