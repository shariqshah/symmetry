PROJECT_NAME    = Symmetry
CC              = gcc
SRC_DIR         = ../../src
INCLUDE_DIR     = ../../include
DIST_DIR        = ../../bin/linux
BUILD_TYPE      = release
CFLAGS          = -Wall -I$(INCLUDE_DIR) -DUSE_GLAD -std=c99
CFLAGS_GAME     = $(CFLAGS) -DGAME $(shell pkg-config --cflags-only-other sdl2 openal)
CFLAGS_GAME_LIB = $(CFLAGS) -DGAME_LIB
CFLAGS_DEBUG    = -g -DGL_DEBUG_CONTEXT -DAL_DEBUG
CFLAGS_RELEASE  = -O3 -ffast-math
LFLAGS          = $(shell pkg-config --libs sdl2 openal) -lm
ITCH_BUTLER     = ~/.config/itch/bin/butler

SRCS_GAME         = main.c platform.c log.c array.c utils.c config_vars.c file_io.c glad.c hashmap.c sound.c string_utils.c variant.c linmath.c
OBJS_RELEASE_GAME = $(patsubst %.c,.release/%.o,$(SRCS_GAME))
OBJS_DEBUG_GAME   = $(patsubst %.c,.debug/%.o,$(SRCS_GAME))

SRCS_GAME_LIB         = gl_load.c glad.c log.c array.c utils.c hashmap.c string_utils.c variant.c linmath.c entity.c game.c transform.c camera.c renderer.c geometry.c light.c bounding_volumes.c editor.c framebuffer.c gui.c input.c material.c model.c scene.c shader.c texture.c
OBJS_RELEASE_GAME_LIB = $(patsubst %.c,.release/%.lib.o,$(SRCS_GAME_LIB))
OBJS_DEBUG_GAME_LIB   = $(patsubst %.c,.debug/%.lib.o,$(SRCS_GAME_LIB))

all: release

library: CFLAGS_GAME_LIB += $(CFLAGS_DEBUG)
library: $(OBJS_RELEASE_GAME_LIB)
	-@echo -e "\e[34mBuilding Game Lib\e[0m"
	-@$(CC) -shared $(OBJS_RELEASE_GAME_LIB) -lm -o lib$(PROJECT_NAME).so

.release/%.lib.o: $(SRC_DIR)/%.c
	-@echo -e "\e[96mCompiling $<\e[0m"
	-@$(CC) -fpic $(CFLAGS_GAME_LIB) -c $< -o $@


release: BUILD_TYPE  = release
release: CFLAGS_GAME += $(CFLAGS_RELEASE)
release: pre_build $(OBJS_RELEASE_GAME) post_build
	-@echo -e "\e[34mLINKING\e[0m Release build"
	-@$(CC) $(OBJS_RELEASE_GAME) $(LFLAGS) -o $(PROJECT_NAME)
	-@echo -e "\e[32mDONE\e[0m building $(PROJECT_NAME)"

debug: BUILD_TYPE  = debug
debug: CFLAGS_GAME += $(CFLAGS_DEBUG)
debug: pre_build $(OBJS_DEBUG_GAME) post_build
	-@echo -e "\e[34mLINKING\e[0m Debug build"
	-@$(CC) $(OBJS_DEBUG_GAME) $(LFLAGS) -o $(PROJECT_NAME)
	-@echo -e "\e[32mDONE\e[0m building $(PROJECT_NAME)"

.release/%.o: $(SRC_DIR)/%.c
	-@echo -e "\e[96mCompiling $<\e[0m"
	-@$(CC) $(CFLAGS_GAME) -c $< -o $@

.debug/%.o: $(SRC_DIR)/%.c
	-@echo -e "\e[96mCompiling $<\e[0m"
	-@$(CC) $(CFLAGS_GAME) -c $< -o $@

pre_build:
	-@mkdir -p .debug .release

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
	-rm -rf .release/*.o
	-rm -rf .debug/*.o
	-@echo -e "\e[0m"

clean_library:
	-@echo -e "\e[31m"
	-rm -f  lib$(PROJECT_NAME).so
	-rm -rf .release/*.lib.o
	-rm -rf .debug/*.lib.o
	-@echo -e "\e[0m"
