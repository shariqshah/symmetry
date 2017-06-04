PROJECT_NAME    = Symmetry
CC              = gcc
SRC_DIR         = ../../src
INCLUDE_DIR     = ../../include
DIST_DIR        = ../../bin/linux
BUILD_TYPE      = release
SRCS            = $(patsubst $(SRC_DIR)/%.c, %.c, $(wildcard ../../src/*.c))
OBJS_RELEASE    = $(patsubst %.c,.release/%.o,$(SRCS))
OBJS_DEBUG      = $(patsubst %.c,.debug/%.o,$(SRCS))
CFLAGS          = -Wall -I$(INCLUDE_DIR) -DUSE_GLAD -std=c99 $(shell pkg-config --cflags-only-other sdl2 openal)
CFLAGS_DEBUG    = -g -DGL_DEBUG_CONTEXT -DAL_DEBUG
CFLAGS_RELEASE  = -O3 -ffast-math -g
LFLAGS          = $(shell pkg-config --libs sdl2 openal) -lm
ITCH_BUTLER     = ~/.config/itch/bin/butler

all: release

release: BUILD_TYPE  = release
release: CFLAGS     += $(CFLAGS_RELEASE)
release: pre_build $(OBJS_RELEASE) post_build
	-@echo -e "\e[34mLINKING\e[0m Release build"
	-@$(CC) $(OBJS_RELEASE) $(LFLAGS) -o $(PROJECT_NAME)
	-@echo -e "\e[32mDONE\e[0m building $(PROJECT_NAME)"

debug: BUILD_TYPE  = debug
debug: CFLAGS     += $(CFLAGS_DEBUG)
debug: pre_build $(OBJS_DEBUG) post_build
	-@echo -e "\e[34mLINKING\e[0m Debug build"
	-@$(CC) $(OBJS_DEBUG) $(LFLAGS) -o $(PROJECT_NAME)
	-@echo -e "\e[32mDONE\e[0m building $(PROJECT_NAME)"

.release/%.o: $(SRC_DIR)/%.c
	-@echo -e "\e[96mCompiling $<\e[0m"
	-@$(CC) $(CFLAGS) -c $< -o $@

.debug/%.o: $(SRC_DIR)/%.c
	-@echo -e "\e[96mCompiling $<\e[0m"
	-@$(CC) $(CFLAGS) -c $< -o $@

pre_build:
	-@mkdir .debug .release

post_build:
	-@ln -fs /mnt/Dev/Projects/symmetry/assets assets

dist: release
	-@echo -e "\e[34mCreating Distribution at $(DIST_DIR)/$(PROJECT_NAME)\e[0m"
	-@strip $(PROJECT_NAME)
	-@rm -rf $(DIST_DIR)/$(PROJECT_NAME)
	-@mkdir -p $(DIST_DIR)/$(PROJECT_NAME)
	-@mkdir -p $(DIST_DIR)/$(PROJECT_NAME)/lib
	-@cp -R ../../assets $(DIST_DIR)/$(PROJECT_NAME)
	-@cp /usr/lib64/libSDL2* $(DIST_DIR)/$(PROJECT_NAME)/lib
	-@cp /usr/lib64/libopenal* $(DIST_DIR)/$(PROJECT_NAME)/lib
	-@echo '#!/bin/sh' >> $(DIST_DIR)/$(PROJECT_NAME)/Launch.sh
	-@echo 'export LD_LIBRARY_PATH=lib' >> $(DIST_DIR)/$(PROJECT_NAME)/Launch.sh
	-@echo './Symmetry' >> $(DIST_DIR)/$(PROJECT_NAME)/Launch.sh
	-@chmod +x $(DIST_DIR)/$(PROJECT_NAME)/Launch.sh
	-@cp $(PROJECT_NAME) $(DIST_DIR)/$(PROJECT_NAME)
	-@chmod +x $(DIST_DIR)/$(PROJECT_NAME)/$(PROJECT_NAME)
	-@echo -e "\e[32mDONE creating distribution\e[0m"

upload_itch: dist
	-@echo -e "\e[34mUploading Linux build to Itch\e[0m"
	-@${ITCH_BUTLER} push ${DIST_DIR}/${PROJECT_NAME}/ bluerriq/symmetry:linux-prealpha
	-@echo -e "\e[32mDONE uploading to Itch\e[0m"

clean:
	-@echo -e "\e[31m"
	-rm -f  $(PROJECT_NAME)
	-rm -rf .release
	-rm -rf .debug
	-@echo -e "\e[0m"
