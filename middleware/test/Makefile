UTIL_DIR = ../util

CC = gcc
CFLAGS = -g -Wall -DDEBUG -I$(UTIL_DIR)
LDFLAGS = -L$(UTIL_DIR)
LDLIBS = -lmls_util

OUT1 = test_log
SRCS1 = test_log.c
OBJS1 = $(SRCS1:.c=.o)

OUT2 = test_conf
SRCS2 = test_conf.c
OBJS2 = $(SRCS2:.c=.o)

OUT3 = test_mserver
SRCS3 = test_mserver.c
OBJS3 = $(SRCS3:.c=.o)

OUT4 = test_mclient
SRCS4 = test_mclient.c
OBJS4 = $(SRCS4:.c=.o)

OUT5 = test_dlink
SRCS5 = test_dlink.c
OBJS5 = $(SRCS5:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: $(OUT1) $(OUT2) $(OUT3) $(OUT4) $(OUT5)
$(OUT1): $(OBJS1)
	$(CC) $(LDFLAGS) -o $(OUT1) $(OBJS1) $(LDLIBS)
$(OUT2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $(OUT2) $(OBJS2) $(LDLIBS)
$(OUT3): $(OBJS3)
	$(CC) $(LDFLAGS) -o $(OUT3) $(OBJS3) $(LDLIBS)
$(OUT4): $(OBJS4)
	$(CC) $(LDFLAGS) -o $(OUT4) $(OBJS4) $(LDLIBS)
$(OUT5): $(OBJS5)
	$(CC) $(LDFLAGS) -o $(OUT5) $(OBJS5)

clean:
	rm -f *.o $(OUT1) $(OUT2) $(OUT3) $(OUT4) $(OUT5)
