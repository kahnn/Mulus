INCDIR = ../include
LIBDIR = ../lib

CC = gcc
CFLAGS = -g -Wall -DDEBUG -I$(INCDIR)
LDFLAGS = 

OUT = libmls_comm.a
SRCS = mls_el.c mls_elope.c mls_elnet.c mls_node.c mls_obj.c
OBJS = $(SRCS:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: install

install: $(OUT)
	mv $(OUT) $(LIBDIR)/

$(OUT): $(OBJS)
	ar r $(OUT) $(OBJS)

clean:
	rm -f $(OUT) $(OBJS)
