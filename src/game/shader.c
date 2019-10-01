#include "shader.h"
#include "../common/array.h"
#include "../common/num_types.h"
#include "../common/string_utils.h"
#include "../common/log.h"
#include "renderer.h"
#include "texture.h"
#include "gl_load.h"
#include "../system/file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_INCLUDE_LINE_LEN 256

static uint* shader_list;
static int*  empty_indices;
static const char* GLSL_VERSION_STR = "#version 330\n";

void debug_print_shader(const char* shaderText)
{
	size_t len = strlen(shaderText);
	int line_count = 1;
	log_raw("%d. ", line_count);
	for(uint i = 0; i < len; i++)
	{
		if(shaderText[i] != '\n')
			log_raw("%c", shaderText[i]);
		else
			log_raw("\n%d. ", ++line_count);
	}
	log_raw("\n END_DEBUG_PRINT\n\n");
}

char* run_preprocessor(char* shader_text, const char* custom_defines)
{
	char* include_loc = strstr(shader_text, "//include");
	if(include_loc)
	{
		char inc_line[MAX_INCLUDE_LINE_LEN];
		memset(inc_line, '\0', MAX_INCLUDE_LINE_LEN);

		char fmt_str[64];
		snprintf(fmt_str, 64, "//include %%%d[^\r\n]", MAX_INCLUDE_LINE_LEN);
		sscanf(shader_text, fmt_str, inc_line);

		char* filename = strtok(inc_line, " ");
		while(filename)
		{
			if(filename)
			{
				char* path = str_new("shaders/");
				path = str_concat(path, filename);
                char* file_contents = io_file_read(DIRT_INSTALL, path, "rb", NULL);
				if(file_contents)
				{
					char* shader_text_new = str_new("%s\n%s", file_contents, shader_text);
					free(shader_text);
					free(file_contents);
					free(path);
					shader_text = shader_text_new;
				}
			}
			filename = strtok(NULL, " ");
		}
	}
    
    //If there are other #defines, add them too
    if(custom_defines)
    {
        char* shader_text_with_custom_defines = str_new("%s\n%s", custom_defines, shader_text);
        free(shader_text);
        shader_text = shader_text_with_custom_defines;
    }
    
    // Insert #version line at the top
    char* shader_text_with_version = str_new("%s\n%s", GLSL_VERSION_STR, shader_text);
    free(shader_text);
    shader_text = shader_text_with_version;
    
	return shader_text;
}

void shader_init(void)
{
	shader_list = array_new(uint);
	empty_indices = array_new(int);
}
	
int shader_create(const char* vert_shader_name, const char* frag_shader_name, const char* custom_defines)
{
	char* vs_path = str_new("shaders/");
	vs_path = str_concat(vs_path, vert_shader_name);
	char* fs_path = str_new("shaders/");
	fs_path = str_concat(fs_path, frag_shader_name);
		
	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    char* vert_source = io_file_read(DIRT_INSTALL, vs_path, "rb", NULL);
    char* frag_source = io_file_read(DIRT_INSTALL, fs_path, "rb", NULL);

	assert(vert_source != NULL);
	assert(frag_source != NULL);

	vert_source = run_preprocessor(vert_source, custom_defines);
	frag_source = run_preprocessor(frag_source, custom_defines);
		
    const char* vert_sourcePtr = vert_source;
    const char* frag_sourcePtr = frag_source;
		
	GL_CHECK(glShaderSource(vert_shader, 1, &vert_sourcePtr, NULL));
	GL_CHECK(glShaderSource(frag_shader, 1, &frag_sourcePtr, NULL));

	GL_CHECK(glCompileShader(vert_shader));
	GL_CHECK(glCompileShader(frag_shader));
		
	GLint is_vert_compiled = 0;
	GLint is_frag_compiled = 0;
	GL_CHECK(glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &is_vert_compiled));
	GL_CHECK(glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &is_frag_compiled));

	if(!is_vert_compiled)
	{
		GLint log_size = 0;
		GL_CHECK(glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &log_size));
		char* message = (char *)malloc(sizeof(char) * log_size);
		GL_CHECK(glGetShaderInfoLog(vert_shader, log_size, NULL, message));

		log_error("shader:create", "COMPILING VS %s : %s", vert_shader_name, message);
		debug_print_shader(vert_source);
		free(message);
	}

	if(!is_frag_compiled)
	{
		GLint log_size = 0;
		GL_CHECK(glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &log_size));
		char* message = (char *)malloc(sizeof(char) * log_size);
		GL_CHECK(glGetShaderInfoLog(frag_shader, log_size, NULL, message));

		log_error("shader:create", "COMPILING FS %s : %s", frag_shader_name, message);
		debug_print_shader(frag_source);
		free(message);
	}

	free(vert_source);
	free(frag_source);
		
	if(!is_vert_compiled || !is_frag_compiled)
	{
		GL_CHECK(glDeleteShader(vert_shader));
		GL_CHECK(glDeleteShader(frag_shader));
		return -1;
	}

	GLuint program = glCreateProgram();
	GL_CHECK(glAttachShader(program, vert_shader));
	GL_CHECK(glAttachShader(program, frag_shader));

	// Bind attribute locations
	GL_CHECK(glBindAttribLocation(program, ATTRIB_LOC_POSITION, "vPosition"));
	GL_CHECK(glBindAttribLocation(program, ATTRIB_LOC_NORMAL,   "vNormal"));
	GL_CHECK(glBindAttribLocation(program, ATRRIB_LOC_UV,       "vUV"));
	//GL_CHECK(glBindAttribLocation(program, ATTRIB_LOC_COLOR,    "vColor"));
	GL_CHECK(glLinkProgram(program));

	GLint is_linked = 0;
	GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &is_linked));
	if(is_linked != GL_TRUE)
	{
		GLint log_size = 0;
		GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size));
		char* message = (char *)malloc(sizeof(char) * log_size);
		GL_CHECK(glGetProgramInfoLog(program, log_size, NULL, message));
		log_error("shader:create", "LINK SHADER : %s", message);
		free(message);

		GL_CHECK(glDeleteProgram(program));
		GL_CHECK(glDeleteShader(vert_shader));
		GL_CHECK(glDeleteShader(frag_shader));
			
		return -1;
	}

	/* Safe to delete shaders now */
	GL_CHECK(glDeleteShader(vert_shader));
	GL_CHECK(glDeleteShader(frag_shader));
	
	/* add new object or overwrite existing one */
	uint* new_shader = 0;
	int index = -1;
	int empty_len = array_len(empty_indices);
	if(empty_len != 0)
	{
		index = empty_indices[empty_len - 1];
		array_pop(empty_indices);
		new_shader = &shader_list[index];
	}
	else
	{
		new_shader = array_grow(shader_list, uint);
		index = array_len(shader_list) - 1;
	}
	assert(new_shader);
	*new_shader = program;
	
	log_message("%s, %s compiled into shader program", vert_shader_name, frag_shader_name);
	free(vs_path);
	free(fs_path);
	
	return index;
}

void shader_bind(const int shader_index)
{
	GL_CHECK(glUseProgram(shader_list[shader_index]));
}

void shader_unbind(void)
{
	GL_CHECK(glUseProgram(0));
}

int shader_get_uniform_location(const int shader_index, const char* name)
{
	GLint handle = 0;
	GL_CHECK(handle = glGetUniformLocation(shader_list[shader_index], name));
	if(handle == -1)
		log_error("shader:get_uniform_location", "Invalid uniform %s", name);

	return handle;
}

void shader_set_uniform_int(const int shader_index, const char* name, const int value)
{
	GLint location = shader_get_uniform_location(shader_index, name);
	if(location >= 0)
		GL_CHECK(glUniform1i(location, value));
}
	
void shader_set_uniform_float(const int shader_index, const char* name, const float value)
{
	GLint location = shader_get_uniform_location(shader_index, name);
	if(location >= 0)
		GL_CHECK(glUniform1f(location, value));
}
	
void shader_set_uniform_vec2(const int shader_index,  const char* name, const vec2* value)
{
	GLint location = shader_get_uniform_location(shader_index, name);
	if(location >= 0)
		GL_CHECK(glUniform2fv(location, 1, &value->x));
}
	
void shader_set_uniform_vec3(const int shader_index,  const char* name, const vec3* value)
{
	GLint location = shader_get_uniform_location(shader_index, name);
	if(location >= 0)
		GL_CHECK(glUniform3fv(location, 1, &value->x));
}
	
void shader_set_uniform_vec4(const int shader_index,  const char* name, const vec4* value)
{
	GLint location = shader_get_uniform_location(shader_index, name);
	if(location >= 0)
		GL_CHECK(glUniform4fv(location, 1, &value->x));
}
	
void shader_set_uniform_mat4(const int shader_index,  const char* name, const mat4* value)
{
	GLint location = shader_get_uniform_location(shader_index, name);
	if(location >= 0)
		GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, &value->mat[0]));
}

void shader_remove(const int shader_index)
{
	uint shader = shader_list[shader_index];
	if(shader == 0) return; 	/* shader is already deleted or invalid */
	int curr_program = 0;
	GL_CHECK(glGetIntegerv(GL_CURRENT_PROGRAM, &curr_program));
	if((uint)curr_program == shader)
		GL_CHECK(glUseProgram(0));
	GL_CHECK(glDeleteProgram(shader));
	shader_list[shader_index] = 0;
	array_push(empty_indices, shader_index, int);
}
	
void shader_cleanup(void)
{
	for(int i = 0; i < array_len(shader_list); i++)
		shader_remove(i);

	array_free(shader_list);
	array_free(empty_indices);
	shader_list = NULL;
	empty_indices = NULL;
}

void shader_set_uniform(const int uniform_type, const int uniform_loc, void* value)
{
	assert(value);
	switch(uniform_type)
	{
	case UT_INT:
	{
		GL_CHECK(glUniform1i(uniform_loc, *((int*)value)));
		break;
	}
	case UT_FLOAT:
	{
		GL_CHECK(glUniform1f(uniform_loc, *((float*)value)));
		break;
	}
	case UT_VEC2:
	{
		vec2* vector = (vec2*)value;
		GL_CHECK(glUniform2fv(uniform_loc, 1, &vector->x));
		break;
	}
	case UT_VEC3:
	{
		vec3* vector = (vec3*)value;
		GL_CHECK(glUniform3fv(uniform_loc, 1, &vector->x));
		break;
	}
	case UT_VEC4:
	{
		vec4* vector = (vec4*)value;
		GL_CHECK(glUniform4fv(uniform_loc, 1, &vector->x));
		break;
	}
	case UT_MAT4:
	{
		mat4* mat = (mat4*)value;
		GL_CHECK(glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, &mat->mat[0]));
		break;
	}
	case UT_TEX:
	{
		int texture_index = *((int*)value);
		int texture_unit = texture_get_textureunit(texture_index);
		GL_CHECK(glUniform1i(uniform_loc, (GL_TEXTURE0 + texture_unit) - GL_TEXTURE0));
		texture_bind(texture_index);
		break;
	}
	}
}

int shader_get_attribute_location(const int shader_index, const char* attrib_name)
{
	assert(shader_index > -1 && shader_index < array_len(shader_list));
	int location = 0;
	GL_CHECK(location = glGetAttribLocation(shader_list[shader_index], attrib_name));
	return location;
}
