#ifndef FILE_IO_H
#define FILE_IO_H

#include "../common/common.h"

void  io_file_init(const char* install_dir, const char* user_dir);
void  io_file_cleanup(void);
char* io_file_read(const int directory_type, const char* path, const char* mode, long* file_size);
FILE* io_file_open(const int directory_type, const char* path, const char* mode);
bool  io_file_copy(const int directory_type, const char* source, const char* destination);
bool  io_file_delete(const int directory_type, const char* filename);

#endif
