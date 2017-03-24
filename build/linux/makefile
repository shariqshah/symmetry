PROJECT_NAME    = Symmetry
CC              = gcc
SRC_DIR         = ../../src
INCLUDE_DIR     = ../../include
LIB_DIR_RELEASE = ../../libs/release/linux64_gcc
LIB_DIR_DEBUG   = ../../libs/debug/linux64_gcc
BUILD_TYPE      = release
SRCS            = $(patsubst $(SRC_DIR)/%.c, %.c, $(wildcard ../../src/*.c))
OBJS            = $(patsubst %.c,%.o,$(SRCS))
OBJS_RELEASE    = $(patsubst %.c,.release/%.o,$(SRCS))
OBJS_DEBUG      = $(patsubst %.c,.debug/%.o,$(SRCS))
CFLAGS          = -Wall -I$(INCLUDE_DIR) -DUSE_GLAD
CFLAGS_DEBUG    = -g -DGL_DEBUG_CONTEXT -DAL_DEBUG -std=c99
CFLAGS_RELEASE  = -O2
LFLAGS          = -lSDL2 -lpthread -ldl -lm -lopenal -lsndio -lGL

all: release

release: BUILD_TYPE  = release
release: CFLAGS     += $(CFLAGS_RELEASE)
release: pre_build $(OBJS) post_build
	$(CC) $(OBJS_RELEASE) -L$(LIB_DIR_RELEASE) $(LFLAGS) -o $(PROJECT_NAME)

debug: BUILD_TYPE  = debug
debug: CFLAGS     += $(CFLAGS_DEBUG)
debug: pre_build $(OBJS) post_build
	$(CC) $(OBJS_DEBUG) -L$(LIB_DIR_DEBUG) $(LFLAGS) -o $(PROJECT_NAME)

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o .$(BUILD_TYPE)/$@

pre_build:
	-@mkdir .debug .release

post_build:
	-@ln -fs /mnt/Dev/Projects/symmetry/assets assets

clean:
	-rm -f  $(PROJECT_NAME)
	-rm -rf .release
	-rm -rf .debug
