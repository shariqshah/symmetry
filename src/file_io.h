#ifndef file_io_H
#define file_io_H

#include <stdio.h>

void  io_file_init(const char* assets_path);
char* io_file_read(const char* path);
FILE* io_file_open(const char* path, const char* mode);
void  io_file_cleanup(void);

#endif
