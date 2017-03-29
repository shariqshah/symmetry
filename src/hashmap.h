#ifndef HASHMAP_H
#define HASHMAP_H

#include "linmath.h"

struct Hashmap;
struct Variant;

struct Hashmap* 	  hashmap_new(void);
void            	  hashmap_free(struct Hashmap* hashmap);
void            	  hashmap_value_remove(struct Hashmap* hashmap, const char* key);
void            	  hashmap_value_set(struct Hashmap* hashmap, const char* key, const struct Variant* value);
const struct Variant* hashmap_value_get(struct Hashmap* hashmap, const char* key);

void     	  		  hashmap_float_set(struct Hashmap* hashmap, const char* key, float value);
void     	  		  hashmap_int_set(struct Hashmap* hashmap, const char* key, int value);
void     	  		  hashmap_double_set(struct Hashmap* hashmap, const char* key, double value);
void     	  		  hashmap_bool_set(struct Hashmap* hashmap, const char* key, int value);
void     	  		  hashmap_vec2_set(struct Hashmap* hashmap, const char* key, const vec2* value);
void     	  		  hashmap_vec3_set(struct Hashmap* hashmap, const char* key, const vec3* value);
void     	  		  hashmap_vec4_set(struct Hashmap* hashmap, const char* key, const vec4* value);
void     	  		  hashmap_quat_set(struct Hashmap* hashmap, const char* key, const quat* value);
void     	  		  hashmap_mat4_set(struct Hashmap* hashmap, const char* key, const mat4* value);
void     	  		  hashmap_str_set(struct Hashmap* hashmap, const char* key, const char* value);
void     	  		  hashmap_ptr_set(struct Hashmap* hashmap, const char* key, void* value);

float           	  hashmap_float_get(struct Hashmap* hashmap, const char* key);
int             	  hashmap_int_get(struct Hashmap* hashmap, const char* key);
double          	  hashmap_double_get(struct Hashmap* hashmap, const char* key);
int             	  hashmap_get_bool(struct Hashmap* hashmap, const char* key);
vec2            	  hashmap_vec2_get(struct Hashmap* hashmap, const char* key);
vec3            	  hashmap_vec3_get(struct Hashmap* hashmap, const char* key);
vec4            	  hashmap_vec4_get(struct Hashmap* hashmap, const char* key);
quat            	  hashmap_quat_get(struct Hashmap* hashmap, const char* key);
const mat4*     	  hashmap_mat4_get(struct Hashmap* hashmap, const char* key);
const char*     	  hashmap_str_get(struct Hashmap* hashmap, const char* key);
void*           	  hashmap_ptr_get(struct Hashmap* hashmap, const char* key);

void                  hashmap_debug_print(struct Hashmap* hashmap);

#endif
