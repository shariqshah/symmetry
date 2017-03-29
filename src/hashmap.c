#include "hashmap.h"
#include "array.h"
#include "variant.h"
#include "log.h"
#include "string_utils.h"

#include <stdlib.h>
#include <string.h>

#define HASH_MAP_NUM_BUCKETS 10
#define HASH_MAX_KEY_LEN     128

struct Hashmap_Entry
{
	char* key;
	struct Variant value;
};

struct Hashmap
{
	struct Hashmap_Entry* buckets[HASH_MAP_NUM_BUCKETS];
};

static unsigned int          hashmap_generate_hash(const char* key);
static struct Hashmap_Entry* hashmap_entry_new(struct Hashmap* hashmap, const char* key);

static struct Hashmap_Entry* hashmap_entry_new(struct Hashmap* hashmap, const char* key)
{
	unsigned int index = hashmap_generate_hash(key);
	struct Hashmap_Entry* new_entry = NULL;
	for(int i = 0; i < array_len(hashmap->buckets[index]); i++) /* Look for duplicates and over-write if found */
	{
		if(strncmp(key, hashmap->buckets[index][i].key, HASH_MAX_KEY_LEN) == 0)
		{
			new_entry = &hashmap->buckets[index][i];
			if(new_entry->key) free(new_entry->key);
			break;
		}
	}
	if(!new_entry) new_entry = array_grow(hashmap->buckets[index], struct Hashmap_Entry);
	new_entry->key = str_new(key);
	return new_entry;
}

unsigned int hashmap_generate_hash(const char* key)
{
	unsigned int index      = 0;
	const int    multiplier = 51;
	for(int i = 0; i < (int)strlen(key); i++)
		index = index * multiplier + key[i];
	return index % HASH_MAP_NUM_BUCKETS;
}

struct Hashmap* hashmap_new(void)
{
	struct Hashmap* hashmap = malloc(sizeof(*hashmap));
	if(!hashmap)
		return NULL;
	for(int i = 0; i < HASH_MAP_NUM_BUCKETS; i++)
		hashmap->buckets[i] = array_new(struct Hashmap_Entry);
	return hashmap;
}

void hashmap_free(struct Hashmap* hashmap)
{
	if(!hashmap) return;
	for(int i = 0; i < HASH_MAP_NUM_BUCKETS; i++)
	{
		for(int j = 0; j < array_len(hashmap->buckets[i]); j++)
		{
			struct Hashmap_Entry* entry = &hashmap->buckets[i][j];
			if(entry->key)
			{
				free(entry->key);
				entry->key = NULL;
			}
			variant_free(&entry->value);
		}
		array_free(hashmap->buckets[i]);
		hashmap->buckets[i] = NULL;
	}
	free(hashmap);
	hashmap = NULL;
}

void hashmap_value_set(struct Hashmap* hashmap, const char* key, const struct Variant* value)
{
	if(!hashmap || !key || !value) return;
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_copy(&new_entry->value, value);
}

const struct Variant* hashmap_value_get(struct Hashmap* hashmap, const char* key)
{
	if(!hashmap || !key) return NULL;
	struct Variant* value = NULL;
	unsigned int index = hashmap_generate_hash(key);
	for(int i = 0; i < array_len(hashmap->buckets[index]); i++)
	{
		if(strncmp(key, hashmap->buckets[index][i].key, HASH_MAX_KEY_LEN) == 0)
		{
			value = &hashmap->buckets[index][i].value;
			break;
		}
	}
	return value;
}

void hashmap_value_remove(struct Hashmap* hashmap, const char* key)
{
	if(!hashmap || !key) return;
	unsigned int index = hashmap_generate_hash(key);
	int index_to_remove = -1;
	for(int i = 0; i < array_len(hashmap->buckets[index]); i++)
	{
		if(strncmp(key, hashmap->buckets[index][i].key, HASH_MAX_KEY_LEN) == 0)
		{
			index_to_remove = i;
			break;
		}
	}
	if(index_to_remove != -1) array_remove_at(hashmap->buckets[index], index_to_remove);
}


void hashmap_float_set(struct Hashmap* hashmap, const char* key, float value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_float(&new_entry->value, value);
}

void hashmap_int_set(struct Hashmap* hashmap, const char* key, int value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_int(&new_entry->value, value);
}

void hashmap_double_set(struct Hashmap* hashmap, const char* key, double value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_double(&new_entry->value, value);
}

void hashmap_bool_set(struct Hashmap* hashmap, const char* key, int value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_bool(&new_entry->value, value);
}

void hashmap_vec2_set(struct Hashmap* hashmap, const char* key, const vec2* value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_vec2(&new_entry->value, value);
}

void hashmap_vec3_set(struct Hashmap* hashmap, const char* key, const vec3* value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_vec3(&new_entry->value, value);
}

void hashmap_vec4_set(struct Hashmap* hashmap, const char* key, const vec4* value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_vec4(&new_entry->value, value);
}

void hashmap_quat_set(struct Hashmap* hashmap, const char* key, const quat* value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_quat(&new_entry->value, value);
}

void hashmap_mat4_set(struct Hashmap* hashmap, const char* key, const mat4* value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_mat4(&new_entry->value, value);
}

void hashmap_str_set(struct Hashmap* hashmap, const char* key, const char* value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_str(&new_entry->value, value);
}

void hashmap_ptr_set(struct Hashmap* hashmap, const char* key, void* value)
{
	struct Hashmap_Entry* new_entry = hashmap_entry_new(hashmap, key);
	variant_assign_ptr(&new_entry->value, value);
}

float hashmap_float_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_float;
}

int hashmap_int_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_int;
}

double hashmap_double_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_double;
}

int hashmap_get_bool(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_bool;
}

vec2 hashmap_vec2_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_vec2;
}

vec3 hashmap_vec3_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_vec3;
}

vec4 hashmap_vec4_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_vec4;
}

quat hashmap_quat_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_quat;
}

const mat4* hashmap_mat4_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_mat4;
}

const char* hashmap_str_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_str;
}

void* hashmap_ptr_get(struct Hashmap* hashmap, const char* key)
{
	const struct Variant* variant = hashmap_value_get(hashmap, key);
	return variant->val_voidptr;
}

void hashmap_debug_print(struct Hashmap* hashmap)
{
	if(!hashmap) return;
	static char str[128];
	memset(str, '\0', 128);
	for(int i = 0; i < HASH_MAP_NUM_BUCKETS; i++)
	{
		log_message("Bucket : %d", i);
		log_message("Bucket len : %d", array_len(hashmap->buckets[i]));
		for(int j = 0; j < array_len(hashmap->buckets[i]); j++)
		{
			struct Hashmap_Entry* entry   = &hashmap->buckets[i][j];
			const struct Variant* value = &entry->value;
			log_message("Key : %s", entry->key);
			variant_to_str(value, str, 128);
			log_message("Value : %s", str);
			memset(str, '\0', 128);
		}
	}
}
