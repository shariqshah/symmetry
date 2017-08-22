#ifndef HASHMAP_H
#define HASHMAP_H

#include "linmath.h"
#include "num_types.h"
#include "array.h"

#define HASH_MAP_NUM_BUCKETS 10
#define HASH_MAX_KEY_LEN     128

struct Hashmap;
struct Variant;

struct Hashmap* 	  hashmap_new(void);
void            	  hashmap_free(struct Hashmap* hashmap);
void            	  hashmap_value_remove(struct Hashmap* hashmap, const char* key);
void            	  hashmap_value_set(struct Hashmap* hashmap, const char* key, const struct Variant* value);
struct Variant*       hashmap_value_get(const struct Hashmap* hashmap, const char* key);

void     	  		  hashmap_float_set(struct Hashmap* hashmap, const char* key, const float value);
void     	  		  hashmap_int_set(struct Hashmap* hashmap, const char* key, const int value);
void     	  		  hashmap_double_set(struct Hashmap* hashmap, const char* key, const double value);
void     	  		  hashmap_bool_set(struct Hashmap* hashmap, const char* key, const bool value);
void     	  		  hashmap_vec2_set(struct Hashmap* hashmap, const char* key, const vec2* value);
void     	  		  hashmap_vec3_set(struct Hashmap* hashmap, const char* key, const vec3* value);
void     	  		  hashmap_vec4_set(struct Hashmap* hashmap, const char* key, const vec4* value);
void     	  		  hashmap_quat_set(struct Hashmap* hashmap, const char* key, const quat* value);
void     	  		  hashmap_vec2_setf(struct Hashmap* hashmap, const char* key, const float x, const float y);
void     	  		  hashmap_vec3_setf(struct Hashmap* hashmap, const char* key, const float x, const float y, const float z);
void     	  		  hashmap_vec4_setf(struct Hashmap* hashmap, const char* key, const float x, const float y, const float z, const float w);
void     	  		  hashmap_quat_setf(struct Hashmap* hashmap, const char* key, const float x, const float y, const float z, const float w);
void     	  		  hashmap_mat4_set(struct Hashmap* hashmap, const char* key, const mat4* value);
void     	  		  hashmap_str_set(struct Hashmap* hashmap, const char* key, const char* value);
void     	  		  hashmap_ptr_set(struct Hashmap* hashmap, const char* key, void* value);

float           	  hashmap_float_get(const struct Hashmap* hashmap, const char* key);
int             	  hashmap_int_get(const struct Hashmap* hashmap, const char* key);
double          	  hashmap_double_get(const struct Hashmap* hashmap, const char* key);
bool             	  hashmap_bool_get(const struct Hashmap* hashmap, const char* key);
vec2            	  hashmap_vec2_get(const struct Hashmap* hashmap, const char* key);
vec3            	  hashmap_vec3_get(const struct Hashmap* hashmap, const char* key);
vec4            	  hashmap_vec4_get(const struct Hashmap* hashmap, const char* key);
quat            	  hashmap_quat_get(const struct Hashmap* hashmap, const char* key);
const mat4*     	  hashmap_mat4_get(const struct Hashmap* hashmap, const char* key);
const char*     	  hashmap_str_get(const struct Hashmap* hashmap, const char* key);
void*           	  hashmap_ptr_get(const struct Hashmap* hashmap, const char* key);

void                  hashmap_debug_print(const struct Hashmap* hashmap);
/* Only used during iteration */
void                  hashmap_iter_begin(struct Hashmap* hashmap);
int                   hashmap_iter_next(struct Hashmap* hashmap, char** key, struct Variant** value);

#define HASHMAP_FOREACH(hashmap, key, value) \
	hashmap_iter_begin(hashmap); \
	while(hashmap_iter_next(hashmap, &key, &value)) \
			
		

#endif
