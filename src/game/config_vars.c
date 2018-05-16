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

void config_vars_init(void)
{
	cvars = hashmap_new();
	/* Initialize with default values incase there is no config file */
	hashmap_int_set(cvars,   "render_width",                 1024);
	hashmap_int_set(cvars,   "render_height",                768);
	hashmap_int_set(cvars,   "fog_mode",                     0);
	hashmap_vec3_setf(cvars, "fog_color",                    0.9f, 0.2f, 0.2f);
	hashmap_float_set(cvars, "fog_density",                  0.1);
	hashmap_float_set(cvars, "fog_start_dist",               10.f);
	hashmap_float_set(cvars, "fog_max_dist",                 50.f);
	hashmap_vec3_setf(cvars, "ambient_light",                0.1f, 0.1f, 0.1f);
	hashmap_bool_set(cvars,  "msaa_enabled",                 1);
	hashmap_int_set(cvars,   "msaa_levels",                  4);
	hashmap_bool_set(cvars,  "debug_draw_enabled",           true);
	hashmap_bool_set(cvars,  "debug_draw_physics",           true);
    hashmap_int_set(cvars,   "video_driver_linux",           VD_WAYLAND);
	hashmap_int_set(cvars,   "debug_draw_mode",              0);
    hashmap_vec4_setf(cvars, "debug_draw_color",             1.f, 0.f, 0.f, 1.f);
	hashmap_float_set(cvars, "player_move_speed",            10.f);
	hashmap_float_set(cvars, "player_move_speed_multiplier", 2.f);
	hashmap_float_set(cvars, "player_turn_speed",            5.f);
}

void config_vars_cleanup(void)
{
	hashmap_free(cvars);
}

struct Hashmap* config_vars_get(void)
{
	return cvars;
}

bool config_vars_load(const char* filename, int directory_type)
{
	FILE* config_file = io_file_open(directory_type, filename, "rb");
	if(!config_file)
	{
		log_error("config:vars_load", "Could not open %s", filename);
		return false;
	}

    struct Parser* parser = parser_load_objects(config_file, filename);
    if(!parser)
    {
        log_error("config_vars:load", "Failed to load config data from %s", filename);
        fclose(config_file);
        return false;
    }

	bool config_loaded = false;
    for(int i = 0; i < array_len(parser->objects); i++)
    {
        struct Parser_Object* object = &parser->objects[i];
		if(object->type != PO_CONFIG)
		{
			log_warning("Unexpected config object type %s in %s", parser_object_type_to_str(object->type), filename);
			continue;
		}

		config_loaded = true;
		char* key = NULL;
		struct Variant* value = NULL;
		char variant_str[MAX_VARIANT_STR_LEN];
		HASHMAP_FOREACH(object->data, key, value)
		{
			struct Variant* existing_val = hashmap_value_get(cvars, key);
			if(!existing_val)
			{
				log_warning("Unkown key '%s' in config file %s", key, filename);
				continue;
			}

			variant_copy(existing_val, value);
		}
    }

	if(config_loaded) log_message("Loaded config from %s", filename);
	fclose(config_file);
	return config_loaded;
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

	struct Parser* parser = parser_new();
	if(!parser)
	{
		log_error("config_vars:save", "Could not create Parser for %s", filename);
		fclose(config_file);
		return false;
	}

	struct Parser_Object* object = parser_object_new(parser, PO_CONFIG);
	if(!object)
	{
		log_error("config_vars:save", "Could not create Parser_Object for %s", filename);
		parser_free(parser);
		fclose(config_file);
		return false;
	}

	hashmap_copy(cvars, object->data);

	if(!parser_write_objects(parser, config_file, filename))
	{
		log_error("config_vars:save", "Failed to write config to '%s'", filename);
		success = false;
	}

	parser_free(parser);
	fclose(config_file);
	return success;
}
