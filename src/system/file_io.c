#include <assert.h>
#include <stdlib.h>

#include "file_io.h"
#include "../common/log.h"
#include "../common/string_utils.h"

static char* executable_directory = NULL;
static char* install_directory = NULL;
static char* user_directory    = NULL;

static char* relative_path_get(const int directory_type);

void io_file_init(const char* install_dir, const char* user_dir)
{
	executable_directory = str_new("%s", install_dir);
	install_directory    = str_new("%s/assets/", install_dir);
	user_directory       = str_new("%s/", user_dir);
}

void io_file_cleanup(void)
{
	if(install_directory)    free(install_directory);
	if(executable_directory) free(executable_directory);
	if(user_directory)       free(user_directory);
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
	assert(directory_type >= 0 && directory_type < DIRT_COUNT);

	char* relative_path = relative_path_get(directory_type);
	if(!relative_path)
	{
		log_error("io:file_open", "Failed to get relative path for '%s'", path);
		return NULL;
	}
	
	relative_path = str_concat(relative_path, path);
	FILE* file = fopen(relative_path, mode);
	if(!file)
		log_error("io:file_open", "Failed to open file '%s'", relative_path);
	free(relative_path);
	return file;
}

bool io_file_copy(const int directory_type, const char* source, const char* destination)
{
	bool success = true;
	long source_size = 0;
	char* source_contents = io_file_read(directory_type, source, "rb", &source_size);
	if(!source_contents)
	{
		log_error("io:file_copy", "Failed to read source file '%s'", source);
		return false;
	}
	
	FILE* dest_file = io_file_open(directory_type, destination, "wb");
	if(!dest_file)
	{
		log_error("io:file_copy", "Failed to open destination file '%s'", destination);
		return false;
	}

	if(fwrite(source_contents, (size_t)source_size, 1, dest_file) != 1)
	{
		log_error("io:file_copy", "Failed to copy source_contents into destination file '%s'", destination);
		success = false;
	}

	log_message("'%s' copied to '%s'", source, destination);
	free(source_contents);
	fclose(dest_file);
	
	return success;
}

bool io_file_delete(const int directory_type, const char* filename)
{
	char* relative_path = relative_path_get(directory_type);
	relative_path = str_concat(relative_path, filename);

	if(remove(relative_path) != 0)
	{
		log_error("io:file_delete", "Failed to delete '%s'", filename);
		return false;
	}
	else
	{
		log_message("'%s' deleted", filename);
	}
	
	free(relative_path);
	return true;
}

static char* relative_path_get(const int directory_type)
{
	char* relative_path = NULL;
	switch(directory_type)
	{
	case DIRT_USER:       relative_path = str_new(user_directory);           break;
	case DIRT_INSTALL:    relative_path = str_new(install_directory);        break;
	case DIRT_EXECUTABLE: relative_path = str_new(executable_directory);     break;
	default: log_error("io:relative_path_get", "Invalid directory type!"); break;
	};
	return relative_path;
}

