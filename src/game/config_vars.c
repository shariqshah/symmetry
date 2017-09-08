#include "config_vars.h"
#include "../common/string_utils.h"
#include "../common/variant.h"
#include "../common/hashmap.h"
#include "file_io.h"
#include "../common/log.h"
#include "../common/parser.h"
#include "platform.h"

#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 512

static struct Hashmap* cvars = NULL;

static void config_on_parser_assign(const char* key, const char* value, const char* filename, int current_line);

void config_vars_init(void)
{
	cvars = hashmap_new();
	/* Initialize with default values incase there is no config file */
	hashmap_int_set(cvars,   "render_width",       1024);
	hashmap_int_set(cvars,   "render_height",      768);
	hashmap_int_set(cvars,   "fog_mode",           0);
	hashmap_vec3_setf(cvars, "fog_color",          0.9f, 0.2f, 0.2f);
	hashmap_float_set(cvars, "fog_density",        0.1);
	hashmap_float_set(cvars, "fog_start_dist",     10.f);
	hashmap_float_set(cvars, "fog_max_dist",       50.f);
	hashmap_vec3_setf(cvars, "ambient_light",      0.1f, 0.1f, 0.1f);
	hashmap_bool_set(cvars,  "msaa_enabled",       1);
	hashmap_int_set(cvars,   "msaa_levels",        4);
	hashmap_bool_set(cvars,  "debug_draw_enabled", 1);
    hashmap_int_set(cvars,  "video_driver_linux",  VD_WAYLAND);
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

void config_on_parser_assign(const char* key, const char* value, const char* filename, int current_line)
{
	struct Variant* cvar = hashmap_value_get(cvars, key);
	if(!cvar)
	{
		log_warning("Unknown config key '%s' in file %s, line %d", key, filename, current_line);
		return;
	}
	variant_from_str(cvar, value, cvar->type);
}

bool config_vars_load(const char* filename, int directory_type)
{
	bool success = false;
	FILE* config_file = io_file_open(directory_type, filename, "r");
	if(!config_file)
	{
		log_error("config:vars_load", "Could not open %s", filename);
		return success;
	}

	if(!parser_load(config_file, filename, &config_on_parser_assign, false, 0))
	{
		log_error("config_vars:load", "Failed to parse config file %s", filename);
	}
	else
	{
		log_message("Loaded config from %s", filename);
		success = true;
	}

	fclose(config_file);
	return success;
}

bool config_vars_save(const char* filename, int directory_type)
{
	bool success = false;
	FILE* config_file = io_file_open(directory_type, filename, "w");
	if(!config_file)
	{
		log_error("config:vars_save", "Failed to open config file %s for writing");
		return success;
	}

	char* key = NULL;
	struct Variant* value = NULL;
	char variant_str[MAX_VARIANT_STR_LEN];
	HASHMAP_FOREACH(cvars, key, value)
	{
		memset(variant_str, '\0', MAX_VARIANT_STR_LEN);
		variant_to_str(value, variant_str, MAX_VARIANT_STR_LEN);
		fprintf(config_file, "%s: %s\n", key, variant_str);
	}
	log_message("Config file %s written.", filename);
	success = true;
	fclose(config_file);
	return success;
}
