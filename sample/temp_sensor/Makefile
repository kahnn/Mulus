INC_DIR = ../../middleware/include
UTIL_DIR = ../../middleware/util
OBJ_DIR = ../../middleware/object
COMM_DIR = ../../middleware/comm

CC = gcc
CFLAGS = -g -Wall -DDEBUG -I$(UTIL_DIR) -I$(INC_DIR)
LDFLAGS = -L$(UTIL_DIR)
LDLIBS = -lmls_util

OUT = temp_sensor
SRCS = main.c \
	$(COMM_DIR)/mls_el.c $(COMM_DIR)/mls_elnet.c $(COMM_DIR)/mls_elope.c \
	$(COMM_DIR)/mls_node.c $(COMM_DIR)/mls_obj.c \
	$(OBJ_DIR)/profile.c $(OBJ_DIR)/temperature_sensor.c
OBJS = $(SRCS:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: $(OUT) $(UTIL_DIR)/libmls_util.a
$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $(OUT) *.o $(LDLIBS)

clean:
	rm -f *.o $(OUT)
