PROJECT_NAME=Symmetry
CC=gcc
SRC_DIR=../../src
SRCS=$(patsubst $(SRC_DIR)/%.c, %.c, $(wildcard ../../src/*.c))
OBJS=$(patsubst %.c,%.o,$(SRCS))
CFLAGS= -o3 -Wall
LIBS=$(shell pkg-config --static --libs glfw3 glew)

all: $(OBJS)
	echo $(SRCS)
	echo $(OBJS)
	$(CC) $(CFLAGS) -o $(PROJECT_NAME) $^ $(LIBS)

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(PROJECT_NAME)
	rm $(OBJS)
