#ifndef SHADER_H
#define SHADER_H

#include "../common/linmath.h"

// Constants for locations of attributes inside all shaders
enum Attribute_Location
{
	AL_POSITION = 0,
    AL_NORMAL   = 1,
    AL_UV       = 2,
    AL_COLOR    = 3
};

enum Uniform_Type
{
	UT_FLOAT = 0,
	UT_INT,
	UT_VEC3,
	UT_VEC2,
	UT_VEC4,
	UT_MAT4,
	UT_TEX
};

int  shader_create(const char* vert_shader_name, const char* frag_shader_name);
void shader_init(void);
void shader_bind(const int shader_index);
void shader_remove(const int shader_index);
void shader_unbind(void);
void shader_set_uniform_int(const int shader_index, const char* name, const int value);
void shader_set_uniform_float(const int shader_index, const char* name, const float value);
void shader_set_uniform_vec2(const int shader_index,  const char* name, const vec2* value);
void shader_set_uniform_vec3(const int shader_index,  const char* name, const vec3* value);
void shader_set_uniform_vec4(const int shader_index,  const char* name, const vec4* value);
void shader_set_uniform_mat4(const int shader_index,  const char* name, const mat4* value);
void shader_set_uniform(const int uniform_type, const int uniform_loc, void* value);
void shader_cleanup(void);
int  shader_get_uniform_location(const int shader_index, const char* name);
int  shader_get_attribute_location(const int shader_index, const char* attrib_name);

#endif
