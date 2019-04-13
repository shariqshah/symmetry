#ifndef GL_LOAD_H
#define GL_LOAD_H

#ifdef USE_GLAD

#include <glad/glad.h>
#include <stdbool.h>

#else


#define SYMMETRY_GL_LIST \
    /*  ret, name, params */ \
    GLE(void,      LinkProgram,              GLuint program) \
    GLE(void,      GetProgramiv,             GLuint program, GLenum pname, GLint *params) \
    GLE(GLuint,    CreateShader,             GLenum type) \
    GLE(void,      ShaderSource,             GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length) \
    GLE(void,      CompileShader,            GLuint shader) \
    GLE(void,      GetShaderiv,              GLuint shader, GLenum pname, GLint *params) \
    GLE(void,      GetShaderInfoLog,         GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    GLE(void,      DeleteShader,             GLuint shader) \
    GLE(GLuint,    CreateProgram,            void) \
    GLE(void,      AttachShader,             GLuint program, GLuint shader) \
    GLE(void,      DetachShader,             GLuint program, GLuint shader) \
    GLE(void,      UseProgram,               GLuint program) \
    GLE(void,      DeleteProgram,            GLuint program) \
    GLE(void,      GenVertexArrays,          GLsizei n, GLuint *arrays) \
    GLE(void,      BindVertexArray,          GLuint array) \
    GLE(void,      BufferData,               GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) \
    GLE(void,      GenBuffers,               GLsizei n, GLuint *buffers) \
    GLE(void,      BindBuffer,               GLenum target, GLuint buffer) \
    GLE(void,      DeleteBuffers,            GLsizei n, const GLuint *buffers) \
    GLE(void,      BindAttribLocation,       GLuint program, GLuint index, const GLchar *name) \
    GLE(GLint,     GetUniformLocation,       GLuint program, const GLchar *name) \
	GLE(void,      Uniform1i,                GLint location, GLint v0)	\
	GLE(void,      Uniform1f,                GLint location, GLfloat v0)	\
	GLE(void,      Uniform2fv,               GLint location, GLsizei count, const GLfloat* value) \
	GLE(void,      Uniform3fv,               GLint location, GLsizei count, const GLfloat* value) \
    GLE(void,      Uniform4f,                GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
    GLE(void,      Uniform4fv,               GLint location, GLsizei count, const GLfloat *value) \
    GLE(void,      UniformMatrix4fv,         GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GLE(void,      DeleteVertexArrays,       GLsizei n, const GLuint *arrays) \
    GLE(void,      EnableVertexAttribArray,  GLuint index) \
    GLE(void,      VertexAttribPointer,      GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) \
    GLE(void,      GenFramebuffers,          GLsizei n, GLuint* framebuffers) \
    GLE(void,      GenRenderbuffers,         GLsizei n, GLuint* renderbuffers) \
    GLE(void,      BindFramebuffer,          GLenum target, GLuint framebuffer) \
    GLE(void,      BindRenderbuffer,         GLenum target, GLuint renderbuffer) \
    GLE(void,      RenderbufferStorage,      GLenum target, GLenum internalformat, GLsizei width, GLsizei height) \
    GLE(void,      FramebufferRenderbuffer,  GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) \
    GLE(GLenum,    CheckFramebufferStatus,   GLenum target) \
    GLE(void,      DeleteFramebuffers,       GLsizei n, const GLuint* framebuffers) \
    GLE(void,      DeleteRenderbuffers,      GLsizei n, const GLuint* renderbuffers) \
    GLE(void,      FramebufferTexture2D,     GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
    GLE(void,      GetProgramInfoLog,        GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) \
    /* end */

#define GLE(ret, name, ...) typedef ret APIENTRY name##proc(__VA_ARGS__); extern name##proc * gl##name;
SYMMETRY_GL_LIST
#undef GLE

#endif


#ifdef GL_DEBUG_CONTEXT
	#define GL_CHECK(expression) do { expression; gl_check_error(#expression, __LINE__, __FILE__);} while(false)
#else
	#define GL_CHECK(expression) (expression)
#endif

int  gl_load_library(void);
bool gl_load_extentions(void);
void gl_check_error(const char* expression, unsigned int line, const char* file);
void gl_cleanup(void);

#endif
