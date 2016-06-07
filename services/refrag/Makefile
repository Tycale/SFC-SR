include ../services.mk

SRC = $(wildcard *.c)
EXEC += refrag

all: $(EXEC)

refrag: libseg6 
	$(CC) -o $@ $(SRC) $(FLAGS) $(DEPLIB)