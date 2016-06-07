
CC      = gcc

# Library
LIBDIR   = ../../services/lib/
SEG6DIR  = ../../seg6ctl/libseg6/
NLMEMDIR = ../../seg6ctl/libnlmem/
NL3DIR   = /usr/include/libnl3

FLAGS = -lcunit  -pthread -lcrypto -lssl -lnl-genl-3 -lnl-3
DIR = -I$(SEG6DIR) -I$(NLMEMDIR) -I$(NL3DIR) 

# Sources 
SRC =$(wildcard *.c)
SRC+=$(wildcard $(LIBDIR)*.c)
SRC+=$(NLMEMDIR)nlmem.o $(SEG6DIR)seg6.o


EXEC = test_all
all: $(EXEC)

test_all: $(SRC)
	$(CC) -o $@ -fprofile-arcs -ftest-coverage $(SRC) $(OBJS) $(FLAGS) $(DIR)

clean:
	rm $(EXEC)
