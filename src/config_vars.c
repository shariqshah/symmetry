#include "config_vars.h"
#include "string_utils.h"
#include "variant.h"
#include "hashmap.h"
#include "file_io.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 512

static struct Hashmap* cvars = NULL;

void config_vars_init(void)
{
	cvars = hashmap_new();
	/* Initialize with default values incase there is no config file */
	hashmap_int_set(cvars,   "render_width",       1024);
	hashmap_int_set(cvars,   "render_height",      768);
	hashmap_int_set(cvars,   "fog_mode",           0);
	hashmap_vec3_setf(cvars, "fog_color",          0.5f, 0.2f, 0.2f);
	hashmap_float_set(cvars, "fog_density",        0.1);
	hashmap_float_set(cvars, "fog_start_dist",     10.f);
	hashmap_float_set(cvars, "fog_max_dist",       50.f);
	hashmap_vec3_setf(cvars, "ambient_light",      0.1f, 0.1f, 0.1f);
	hashmap_bool_set(cvars,  "msaa_enabled",       1);
	hashmap_int_set(cvars,   "msaa_levels",        4);
	hashmap_bool_set(cvars,  "debug_draw_enabled", 1);
	hashmap_int_set(cvars,   "debug_draw_mode",    0);
	hashmap_vec4_setf(cvars, "debug_draw_color",   1.f, 0.f, 0.f, 1.f);
}

void config_vars_cleanup(void)
{
	hashmap_free(cvars);
}

struct Hashmap* config_vars_get(void)
{
	return cvars;
}

int config_vars_load(const char* filename)
{
	int success = 0;
	FILE* config_file = io_file_open(filename, "r");
	if(!config_file)
	{
		log_error("config:vars_load", "Could not open %s", filename);
		return success;
	}

	/* Read line by line, ignore comments */
	char key_str[HASH_MAX_KEY_LEN];
	char line_buffer[MAX_LINE_LEN];
	memset(key_str, '\0', HASH_MAX_KEY_LEN);
	memset(line_buffer, '\0', MAX_LINE_LEN);
	int current_line = 0;
	while(fgets(line_buffer, MAX_LINE_LEN - 1, config_file))
	{
		current_line++;
		line_buffer[strcspn(line_buffer, "\r\n")] = '\0';
		
		if(line_buffer[0] == '#' || strlen(line_buffer) == 0)
			continue;

		log_message("Line : %s", line_buffer);
		memset(key_str, '\0', HASH_MAX_KEY_LEN);
		char* value_str = strstr(line_buffer, ":");
		if(!value_str)
		{
			log_warning("Malformed value in config file %s, line %d", filename, current_line);
			continue;
		}
		int key_str_len = value_str - line_buffer;
		strncpy(key_str, line_buffer, key_str_len);
		if(key_str_len >= HASH_MAX_KEY_LEN)
			key_str[HASH_MAX_KEY_LEN - 1] = '\0';
		else
			key_str[key_str_len] = '\0';
		value_str++; /* Ignore the colon(:) */
		
		//char* key = NULL;
		struct Variant* value = hashmap_value_get(cvars, key_str);
		if(!value)
		{
			log_warning("Unknown value in config file %s, line %d", filename, current_line);
			continue;
		}

		variant_from_str(value, value_str, value->type);
		/* HASHMAP_FOREACH(cvars, key, value) */
		/* { */
		/* 	if(strncmp(line_buffer, key, strlen(key)) == 0) */
		/* 	{ */
		/* 		char* value_str = strstr(line_buffer, ":"); */
		/* 		if(value_str) */
		/* 		{ */
		/* 			value_str++; */
		/* 			variant_from_str(value, value_str, value->type); */
		/* 		} */
		/* 		break; */
		/* 	} */
		/* } */
	}
	
	success = 1;
	fclose(config_file);
	return success;
}

void config_vars_save(const char* filename)
{
	
}
