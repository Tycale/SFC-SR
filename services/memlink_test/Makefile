include ../services.mk

SRC = $(wildcard *.c)
EXEC += memlink_test

all: $(EXEC)

memlink_test: memlink_test.c libseg6
	$(CC) -o $@ $(SRC) $(FLAGS) $(DEPLIB)
