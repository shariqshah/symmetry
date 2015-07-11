#ifndef shader_H
#define shader_H

#include "linmath.h"
    
int  shader_create(const char* vert_shader_name, const char* frag_shader_name);
void shader_initialize(void);
void shader_bind(const int shader_index);
void shader_remove(const int shader_index);
void shader_unbind(void);
void shader_set_uniform_int(const int shaderIndex, const char* name, const int value);
void shader_set_uniform_float(const int shaderIndex, const char* name, const float value);
void shader_set_uniform_vec2(const int shaderIndex,  const char* name, const vec2 value);
void shader_set_uniform_vec3(const int shaderIndex,  const char* name, const vec3 value);
void shader_set_uniform_vec4(const int shaderIndex,  const char* name, const vec4 value);
void shader_set_uniform_mat4(const int shaderIndex,  const char* name, const mat4 value);
void shader_cleanup(void);
int  shader_uniform_location_get(const int shader_index, const char* name);

#endif
