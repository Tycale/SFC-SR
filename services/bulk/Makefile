include ../services.mk

SRC = $(wildcard *.c)
EXEC += bulk

all: lib $(EXEC)

bulk: bulk.c libseg6 
	$(CC) -o $@ $(SRC) $(FLAGS) $(DEPLIB)
