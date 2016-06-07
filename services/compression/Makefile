include ../services.mk

LDFLAGS += -lz

SRC = $(wildcard *.c) $(LZ4DIR)/lz4.c $(LZODIR)minilzo.c
EXEC += compression

all: lib $(EXEC)

compression: compression.c lz4 libseg6 lzo 
	$(CC) -o $@ $(SRC) $(FLAGS) $(DEPLIB)
