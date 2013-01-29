INC_DIR = ../../middleware/include
LIB_DIR = ../../middleware/lib
#COMM_DIR = ../../middleware/comm

CC = gcc
CFLAGS = -g -Wall -DDEBUG -I$(INC_DIR)
LDFLAGS = -L$(LIB_DIR)
LDLIBS =  -lmls_comm -lmls_util

OUT = temp_sensor
SRCS = main.c profile.c temperature_sensor.c

#SRCS = main.c profile.c temperature_sensor.c \
#	$(COMM_DIR)/mls_el.c $(COMM_DIR)/mls_elnet.c $(COMM_DIR)/mls_elope.c \
#	$(COMM_DIR)/mls_node.c $(COMM_DIR)/mls_obj.c

OBJS = $(SRCS:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: $(OUT)
$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $(OUT) *.o $(LDLIBS)

clean:
	rm -f *.o $(OUT)
