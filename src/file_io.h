#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>

enum Directory_Type
{
	DT_USER,					/* User directory or preferences directory */
	DT_INSTALL					/* Directory where the game's executable is, usually where the game is installed */
};

void  io_file_init(const char* install_dir, const char* user_dir);
char* io_file_read(const int directory_type, const char* path, const char* mode, long* file_size);
FILE* io_file_open(const int directory_type, const char* path, const char* mode);
void  io_file_cleanup(void);

#endif
