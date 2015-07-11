#include "shader.h"
#include "file_io.h"
#include "array.h"
#include "num_types.h"
#include "string_utils.h"
#include "log.h"
#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

typedef struct
{
	unsigned int vertex_shader;
	unsigned int fragment_shader;
	unsigned int program;
	
} Shader_Object;


// Constants for locations of attributes inside all shaders
const int POSITION_LOC = 0;
const int NORMAL_LOC   = 1;
const int UV_LOC       = 2;
const int COLOR_LOC    = 3;

static Array* shader_list;
static Array* empty_indices;

void debug_print_shader(const char* shaderText)
{
	size_t len = strlen(shaderText);
	int line_count = 1;
	printf("%d. ", line_count);
	for(uint i = 0; i < len; i++)
	{
		if(shaderText[i] != '\n')
			printf("%c", shaderText[i]);
		else
			printf("\n%d. ", ++line_count);
	}
	printf("\n END_DEBUG_PRINT\n\n");
}

char* run_preprocessor(char* shader_text)
{
	char* include_loc = strstr(shader_text, "//include");
	if(include_loc)
	{
		char* line_end = strchr(include_loc, '\n');
		int line_size  = line_end - include_loc;
		char* inc_line = malloc((sizeof(char) * line_size) + 1);
		strncpy(inc_line, include_loc, line_size);
		inc_line[line_size] = '\0';

		char* filename = strtok(inc_line, " ");
		while(filename)
		{
			filename = strtok(NULL, " ");
			if(filename)
			{
				char* path = str_new("shaders/");
				path = str_concat(path, filename);
				char* file = io_file_read(path);
				char* shader_copy = str_new(shader_text);
				char* temp = realloc(shader_text, (strlen(shader_text) + strlen(file) + 2));
				if(temp)
				{
					shader_text = temp;
					strcpy(shader_text, file);
					strcat(shader_text, shader_copy);
				}
				else
				{
					log_warning("Realloc failed in Shader::run_preprocessor");
				}

				free(path);
				free(shader_copy);
				free(file);
			}
		}
		free(inc_line);
	}
	return shader_text;
}

void shader_initialize(void)
{
	shader_list = array_new(Shader_Object);
	empty_indices = array_new(int);
}
	
int shader_create(const char* vert_shader_name, const char* frag_shader_name)
{
	char* vs_path = str_new("shaders/");
	vs_path = str_concat(vs_path, vert_shader_name);
	char* fs_path = str_new("shaders/");
	fs_path = str_concat(fs_path, frag_shader_name);
		
	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

	char* vert_source = io_file_read(vs_path);
	char* frag_source = io_file_read(fs_path);

	assert(vert_source != NULL);
	assert(frag_source != NULL);

	vert_source = run_preprocessor(vert_source);		
	frag_source = run_preprocessor(frag_source);
		
	GLint v_size = (GLint)strlen(vert_source);
	GLint f_size = (GLint)strlen(frag_source);
		
	const char* vert_sourcePtr = vert_source;
	const char* frag_sourcePtr = frag_source;
		
	const GLint* vert_size = &v_size;
	const GLint* frag_size = &f_size;
		
	glShaderSource(vert_shader, 1, &vert_sourcePtr, vert_size);
	glShaderSource(frag_shader, 1, &frag_sourcePtr, frag_size);

	glCompileShader(vert_shader);
	glCompileShader(frag_shader);
		
	GLint is_vert_compiled = 0;
	GLint is_frag_compiled = 0;
	glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &is_vert_compiled);
	glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &is_frag_compiled);

	if(!is_vert_compiled)
	{
		GLint log_size = 0;
		glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &log_size);
		char* message = (char *)malloc(sizeof(char) * log_size);
		glGetShaderInfoLog(vert_shader, log_size, NULL, message);

		log_error("shader:create", "COMPILING VS %s : %s", vert_shader_name, message);
		debug_print_shader(vert_source);
		free(message);
	}

	if(!is_frag_compiled)
	{
		GLint log_size = 0;
		glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &log_size);
		char* message = (char *)malloc(sizeof(char) * log_size);
		glGetShaderInfoLog(frag_shader, log_size, NULL, message);

		log_error("shader:create", "COMPILING FS %s : %s", frag_shader_name, message);
		debug_print_shader(frag_source);
		free(message);
	}

	free(vert_source);
	free(frag_source);
		
	if(!is_vert_compiled || !is_frag_compiled)
	{
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);
		return -1;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);

	// Bind attribute locations
	glBindAttribLocation(program, POSITION_LOC, "vPosition");
	glBindAttribLocation(program, NORMAL_LOC,   "vNormal");
	glBindAttribLocation(program, UV_LOC,       "vUV");
	glBindAttribLocation(program, COLOR_LOC,    "vColor");
	renderer_check_glerror("shader:create");
	glLinkProgram(program);

	GLint is_linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
	if(!is_linked)
	{
		GLint log_size = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &log_size);
		char* message = (char *)malloc(sizeof(char) * log_size);
		glGetProgramInfoLog(program, log_size, NULL, message);
		log_error("shader:create", "LINK SHADER : %s", message);
		free(message);

		glDeleteProgram(program);
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);
			
		return -1;
	}

	/* add new object or overwrite existing one */
	Shader_Object* new_object = NULL;
	int index = -1;
	if(empty_indices->length != 0)
	{
		index = array_get_last_val(empty_indices, int);
		array_pop(empty_indices);
		new_object = array_get(shader_list, index);
	}
	else
	{
		new_object = array_add(shader_list);
		index = shader_list->length - 1;
	}
	assert(new_object);
	new_object->vertex_shader   = vert_shader;
	new_object->fragment_shader = frag_shader;
	new_object->program         = program;
	
	log_message("%s, %s compiled into shader program", vert_shader_name, frag_shader_name);
	free(vs_path);
	free(fs_path);
	
	return index;
}

void shader_bind(const int shader_index)
{
	Shader_Object* shader_object = array_get(shader_list, shader_index);
	glUseProgram(shader_object->program);
}

void shader_unbind(void)
{
	glUseProgram(0);
}

int get_uniform_location(const int shader_index, const char* name)
{
	Shader_Object shader_object = array_get_val(shader_list, Shader_Object, shader_index);
	GLint handle = glGetUniformLocation(shader_object.program, name);

	if(handle == -1)
		log_error("shader:get_uniform_location", "Invalid uniform %s", name);

	return handle;
}

void set_uniform_int(const int shader_index, const char* name, const int value)
{
	GLint location = get_uniform_location(shader_index, name);
	if(location >= 0)
		glUniform1i(location, value);
}
	
void set_uniform_float(const int shader_index, const char* name, const float value)
{
	GLint location = get_uniform_location(shader_index, name);
	if(location >= 0)
		glUniform1f(location, value);
}
	
void set_uniform_vec2(const int shader_index,  const char* name, const vec2 value)
{
	GLint location = get_uniform_location(shader_index, name);
	if(location >= 0)
		glUniform2fv(location, 1, value);
}
	
void set_uniform_vec3(const int shader_index,  const char* name, const vec3 value)
{
	GLint location = get_uniform_location(shader_index, name);
	if(location >= 0)
		glUniform3fv(location, 1, value);
}
	
void set_uniform_vec4(const int shader_index,  const char* name, const vec4 value)
{
	GLint location = get_uniform_location(shader_index, name);
	if(location >= 0)
		glUniform4fv(location, 1, value);
}
	
void setUniformMat4(const int shader_index,  const char* name, const mat4 value)
{
	GLint location = get_uniform_location(shader_index, name);
	if(location >= 0)
		glUniformMatrix4fv(location, 1, GL_FALSE, *value);
}

void shader_remove(const int shader_index)
{
	Shader_Object* shader_object = array_get(shader_list, shader_index);
	glDeleteProgram(shader_object->program);
	glDeleteShader(shader_object->vertex_shader);
	glDeleteShader(shader_object->fragment_shader);
	shader_object->fragment_shader = shader_object->vertex_shader = shader_object->program = -1;
	array_push(empty_indices, shader_index, int);
}
	
void shader_cleanup(void)
{
	for(int i = 0; i < (int)shader_list->length; i++)
		shader_remove(i);

	array_free(shader_list);
	array_free(empty_indices);
}
