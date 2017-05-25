PROJECT_NAME      = Symmetry.exe
SRC_DIR           = ../../src
INCLUDE_DIR       = $(shell mingw64-pkg-config --cflags sdl2 openal) -I../../include
DIST_DIR          = ../../bin/win_mingw
PROJECT_DIST_NAME = $(patsubst %.exe,%, $(PROJECT_NAME))
BUILD_TYPE        = release
SRCS              = $(patsubst $(SRC_DIR)/%.c, %.c, $(wildcard ../../src/*.c))
OBJS_RELEASE      = $(patsubst %.c,.release/%.o,$(SRCS))
CFLAGS            = -Wall -I$(INCLUDE_DIR) -DUSE_GLAD -std=c99
CFLAGS_RELEASE    = -O3 -ffast-math
LFLAGS            = $(shell mingw64-pkg-config --libs --static sdl2 openal)
LIB_BINARY_DIR    = $(patsubst -L%/lib,%/bin, $(shell mingw64-pkg-config --libs-only-L sdl2))

all: release

release: BUILD_TYPE  = release
release: CFLAGS     += $(CFLAGS_RELEASE)
release: pre_build $(OBJS_RELEASE) post_build
	-@echo -e "\e[34mLINKING\e[0m Release build"
	-@$(CC) $(OBJS_RELEASE) $(LFLAGS) -o $(PROJECT_NAME)
	-@echo -e "\e[32mDONE\e[0m building $(PROJECT_NAME)"

.release/%.o: $(SRC_DIR)/%.c
	-@echo -e "\e[96mCompiling $<\e[0m"
	-@$(CC) $(CFLAGS) -c $< -o $@

pre_build:
	-@mkdir .debug .release

post_build:
	-@ln -fs /mnt/Dev/Projects/symmetry/assets assets
	-@cp $(LIB_BINARY_DIR)/SDL2.dll /mnt/Dev/Projects/symmetry/build/win_mingw64
	-@cp $(LIB_BINARY_DIR)/OpenAL32.dll /mnt/Dev/Projects/symmetry/build/win_mingw64

dist: release
	-@echo -e "\e[34mCreating Distribution at $(DIST_DIR)/$(PROJECT_NAME)\e[0m"
	-@mingw-strip $(PROJECT_NAME)
	-@rm -rf $(DIST_DIR)/$(PROJECT_DIST_NAME)
	-@mkdir -p $(DIST_DIR)/$(PROJECT_DIST_NAME)
	-@cp -R ../../assets $(DIST_DIR)/$(PROJECT_DIST_NAME)
	-@cp $(PROJECT_NAME) $(DIST_DIR)/$(PROJECT_DIST_NAME)
	-@cp SDL2.dll $(DIST_DIR)/$(PROJECT_DIST_NAME)
	-@cp OpenAL32.dll $(DIST_DIR)/$(PROJECT_DIST_NAME)
	-@echo -e "\e[32mDONE creating distribution\e[0m"

clean:
	-@echo -e "\e[31m"
	-rm -f  $(PROJECT_NAME)
	-rm -rf .release
	-rm -rf .debug
	-@echo -e "\e[0m"
