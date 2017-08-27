#include <assert.h>
#include <stdlib.h>

#include "file_io.h"
#include "../common/log.h"
#include "../common/string_utils.h"

static char* install_directory = NULL;
static char* user_directory    = NULL;

void io_file_init(const char* install_dir, const char* user_dir)
{
	install_directory = str_new("%s/assets/", install_dir);
	user_directory    = str_new("%s/", user_dir);
}

void io_file_cleanup(void)
{
	if(install_directory) free(install_directory);
	if(user_directory)    free(user_directory);
	install_directory = NULL;
	user_directory    = NULL;
}

char* io_file_read(const int directory_type, const char* path, const char* mode, long* file_size)
{
	FILE* file = io_file_open(directory_type, path, mode);
	char* data = NULL;
	if(!file) return data;
	int rc = fseek(file, 0, SEEK_END);
	if(rc == 0)
	{
		long length = (size_t)ftell(file);
		if(file_size) *file_size = length;
		rewind(file);
		data = calloc(1, (sizeof(char) * length) + 1);
		if(data)
		{
			if(fread(data, length, 1, file) > 0)
			{
				if(data[length] != '\0') data[length] = '\0';
			}
			else
			{
				log_error("io:file_read", "fread failed");
				free(data);
			}
		}
		else
		{
			log_error("io:file_read", "malloc failed");
		}
	}
	else
	{
		log_error("io:file_read", "fseek failed");
	}
	fclose(file);
	return data;
}

FILE* io_file_open(const int directory_type, const char* path, const char* mode)
{
	assert(directory_type >= 0 && directory_type <= DT_INSTALL);
	
	char* relative_path = str_new(directory_type == DT_INSTALL ? install_directory : user_directory);
	relative_path = str_concat(relative_path, path);
	FILE* file = fopen(relative_path, mode);
	if(!file) log_error("io:file", "Failed to open file '%s'", relative_path);
	free(relative_path);
	return file;
}
