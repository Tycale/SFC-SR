include ../services.mk

SRC = $(wildcard *.c)
EXEC += tun_interface

all: $(EXEC)

tun_interface: tun_interface.c libseg6
	$(CC) -o $@ $(SRC) $(FLAGS) $(DEPLIB)
