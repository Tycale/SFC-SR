include ../services.mk

SRC = $(wildcard *.c)
EXEC += count

all: $(EXEC)

count: count.c lzo libseg6
	$(CC) -o $@ $(SRC) $(FLAGS) $(DEPLIB)
