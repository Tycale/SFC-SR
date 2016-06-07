
# FLAGS
CC      = gcc
CFLAGS  = -O3 -Wextra -Wundef -Wshadow -Wcast-align -Wstrict-prototypes -g #-Wall #-std=c99
LDFLAGS = -lpthread -lcrypto -lssl -lnl-3 -lnl-genl-3 


# LIBRARY
LIB      = ../lib/
LZ4DIR   = ../../lz4/lib/
LZODIR   = ../../minilzo-2.09/
LIBSEG6  = ../../seg6ctl
SEG6DIR  = $(LIBSEG6)/libseg6/
NLMEMDIR = $(LIBSEG6)/libnlmem/
NL3DIR   = /usr/include/libnl3

OBJLIB=$(wildcard ${LIB}*.o)
DIRFLAGS = -I$(SEG6DIR) -I$(NLMEMDIR) -I$(LZ4DIR) -I$(LZODIR) -I$(NL3DIR)


# AGGREGATE
FLAGS = $(CFLAGS) $(LDFLAGS)
DEPLIB = $(DIRFLAGS) $(OBJLIB) $(NLMEMDIR)nlmem.o $(SEG6DIR)seg6.o


# EXECUTABLE
EXEC = 

all: $(EXEC)

lib: 
	make -C ${LIB}

libseg6:
	make -C $(LIBSEG6)

lzo:
	make gcc -C $(LZODIR)

lz4:
	make -C $(LZ4DIR)

clean:
	rm -f $(NLMEMDIR)nlmem.o $(SEG6DIR)seg6.o
	rm -f $(EXEC)
	make clean -C $(LIB)
