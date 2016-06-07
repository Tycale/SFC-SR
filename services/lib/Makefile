include ../services.mk

LDFLAGS += -pthread -lcrypto -lssl -lnl-genl-3 -lnl-3 -g


SRCS=$(wildcard *.c)
OBJS = $(SRCS:c=o) 

HDR=$(wildcard *.h)

FLAGS = $(CFLAGS) $(LDFLAGS) $(DIRFLAGS)

all: ${OBJS}

%.o: %.c
	${CC} -c ${SRCS} $(HDR) ${FLAGS} 

clean:
	@rm -f *.o
	@rm -f *.h.gch
