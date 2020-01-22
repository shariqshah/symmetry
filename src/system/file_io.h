#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdbool.h>
#include <stdio.h>

#include "../common/limits.h"

enum Directory_Type
{
	DIRT_USER,		 /* User directory or preferences directory */
	DIRT_INSTALL,	 /* Directory where the game's assets are, usually alongside the game's executable where the game is installed */
	DIRT_EXECUTABLE, /* Directory where the game's executable is located */
	DIRT_COUNT
};

void  io_file_init(const char* install_dir, const char* user_dir);
void  io_file_cleanup(void);
char* io_file_read(const int directory_type, const char* path, const char* mode, long* file_size);
FILE* io_file_open(const int directory_type, const char* path, const char* mode);
bool  io_file_copy(const int directory_type, const char* source, const char* destination);
bool  io_file_delete(const int directory_type, const char* filename);

#endif
