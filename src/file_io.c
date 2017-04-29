#include <assert.h>
#include <stdlib.h>

#include "file_io.h"
#include "log.h"
#include "string_utils.h"

static char* base_assets_path;

void io_file_init(const char* assets_path)
{
	base_assets_path = str_new("%s/assets/", assets_path);
}

void io_file_cleanup(void)
{
	if(base_assets_path) free(base_assets_path);
}

char* io_file_read(const char* path, const char* mode, long* file_size)
{
	FILE* file = io_file_open(path, mode);
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

FILE* io_file_open(const char* path, const char* mode)
{
	char* relative_path = str_new(base_assets_path);
	relative_path = str_concat(relative_path, path);
	FILE* file = fopen(relative_path, mode);
	if(!file) log_error("io:file", "Failed to open file '%s'", relative_path);
	free(relative_path);
	return file;
}
