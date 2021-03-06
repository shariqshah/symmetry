#include "texture.h"
#include "../common/array.h"
#include "../common/string_utils.h"
#include "../common/memory_utils.h"
#include "../common/log.h"
#include "../common/num_types.h"
#include "renderer.h"
#include "gl_load.h"
#include "../system/file_io.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

struct Texture
{
	char*  name;
	uint   handle;
	int    ref_count;
	int    texture_unit;
	GLenum format;
	GLint  internal_format;
	GLenum type;
};

#pragma pack(push, 1)
struct Tga_Header
{
	char  idlength;
	char  colourmaptype;
	char  datatypecode;
	short colourmaporigin;
	short colourmaplength;
	char  colourmapdepth;
	short x_origin;
	short y_origin;
	short width;
	short height;
	char  bitsperpixel;
	char  imagedescriptor;
};
#pragma pack(pop)

static struct Texture* texture_list;
static int* empty_indices;

#define MAX_PIXEL_BYTES 5

static int  load_img(FILE* file, GLubyte** image_data, int* width, int* height, int* fmt, int* internal_format);
static void debug_write_tga(struct Tga_Header* header, GLubyte* image_data);
static void copy_tga_pixel(GLubyte* source, GLubyte* dest, size_t bytes_per_pixel);
static void create_gl_texture(uint* out_handle, int width, int height, int format, int internal_format, int type, const void* data);

void texture_init(void)
{
	texture_list  = array_new(struct Texture);
	empty_indices = array_new(int);
}

int texture_create_from_file(const char* filename, int texture_unit)
{
	assert(filename);
	/* check if texture is already loaded */
	int index = texture_find(filename);
	if(index >= 0)
	{
		texture_list[index].ref_count++;
		return index;
	}
	/* If texture not already loaded then try to load it */
	char* full_path = str_new("textures/%s", filename);
    FILE* file = io_file_open(DIRT_INSTALL, full_path, "rb");
	int img_load_success = -1;
	
	if(file)
	{
		/* Load texture here */
		int width, height, internal_format, fmt;
		GLubyte* img_data = NULL;
		width = height = internal_format = fmt = -1;
		img_load_success = load_img(file, &img_data, &width, &height, &fmt, &internal_format);

		if(img_load_success)
		{
			index = texture_create(filename, texture_unit, width, height, fmt, internal_format, GL_UNSIGNED_BYTE, img_data);
			if(index > -1)
			{
				texture_set_param(index, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				texture_set_param(index, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				texture_set_param(index, GL_TEXTURE_WRAP_S,GL_REPEAT);
				texture_set_param(index, GL_TEXTURE_WRAP_T,GL_REPEAT);
			}
			else
			{
				log_error("texture:create_from_file", "Error creating texture");
			}
			if(img_data) memory_free(img_data);
		}
		fclose(file);
	}
	else
	{
		log_error("texture:create_from_file", "Could not open file %s", filename);
	}
	memory_free(full_path);
	return index;
}

void texture_remove(int index)
{
	if(index > -1 && index < array_len(texture_list))
	{
		struct Texture* texture = &texture_list[index];
		if(texture->ref_count >= 0)
		{
			texture->ref_count--;
			if(texture->ref_count < 0)
			{	
				glDeleteTextures(1, &texture->handle);
				if(texture->name) memory_free(texture->name);
				texture->name            = NULL;
				texture->ref_count       = -1;
				texture->texture_unit    = -1;
				texture->format          = -1;
				texture->internal_format = -1;
				texture->type            = -1;
				array_push(empty_indices, index, int);			
			}
		}
	}
}

int texture_find(const char* name)
{
	assert(name);
	int index = -1;
	for(int i = 0; i < array_len(texture_list); i++)
	{
		struct Texture* texture = &texture_list[i];
		if(texture->name && strcmp(texture->name, name) == 0)
		{
			index = i;
			break;
		}
	}
	return index;
}

void texture_cleanup(void)
{
	for(int i = 0; i < array_len(texture_list); i++)
		texture_remove(i);

	array_free(texture_list);
	array_free(empty_indices);
	texture_list  = NULL;
	empty_indices = NULL;
}

void texture_bind(int index)
{
	if(index < 0 || index >= array_len(texture_list))
	{
		log_warning("Cannot bind invalid texture handle");
		return;
	}
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + texture_list[index].texture_unit));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_list[index].handle));
}

void texture_unbind(int index)
{
	if(index < 0 || index >= array_len(texture_list))
	{
		log_warning("Cannot unbind invalid texture handle");
		return;
	}
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + texture_list[index].texture_unit));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

int load_img(FILE* file, GLubyte** image_data, int* width, int* height, int* fmt, int* internal_format)
{	
	int success = 0;
	struct Tga_Header header; 
	size_t items_read = fread(&header, sizeof(struct Tga_Header), 1, file);

	 if(items_read == 1)
	 {
		 if(header.datatypecode == 0)
		 {
			 log_error("texture:load_img", "No image data in file");
		 }
		 else
		 {
			 /* only compressed and uncompressed true color image data supported yet */
			 if(header.datatypecode != 2 && header.datatypecode != 10)
			 {
				 log_error("texture:load_img", "Unsupported image data type");
				 return success;
			 }

			 if(header.bitsperpixel != 24 && header.bitsperpixel != 32)
			 {
				 log_error("texture:load_img",
						   "Unsupported bitsperpixel size(%d), only 24 and 32 supported", header.bitsperpixel);
				 return success;
			 }

			 if(header.width <= 0 || header.height <= 0)
			 {
				 log_error("texture:load_img", "Invalid width and height (%d:%d)", header.width, header.height);
				 return success;
			 }

			 size_t bytes_per_pixel = header.bitsperpixel / 8;
			 size_t image_size = (bytes_per_pixel * header.width * header.height);
			 *image_data = memory_allocate(image_size);
			 if(!*image_data)
			 {
				 log_error("texture:load_img", "Out of memory");
				 return success;
			 }

			 /* skip over unnecessary things like colormap data */
			 int skipover = 0;
			 skipover += header.idlength;
			 skipover += header.colourmaptype * header.colourmaplength;
			 fseek(file, skipover, SEEK_CUR);

			 /* Start reading pixel by pixel */
			 if(header.datatypecode == 2) /* uncompressed image data */
			 {
				 GLubyte pixel[MAX_PIXEL_BYTES];
				 GLubyte* curr_pixel = *image_data;
				 for(int i = 0; i < header.width * header.height; i++)
				 {
					 if(fread(&pixel, 1, bytes_per_pixel, file) != bytes_per_pixel)
					 {
						 log_error("texture:load_img", "Unexpected end of file at pixel %d", i);
						 memory_free(*image_data);
						 *image_data = NULL;
						 return success;
					 }

					 /* Copy pixel and change BGR to RGB */
					 copy_tga_pixel(&pixel[0], curr_pixel, bytes_per_pixel);
					 curr_pixel += bytes_per_pixel;
				 }
			 }
			 else if(header.datatypecode == 10) /* RLE encoded image data */
			 {
                 size_t chunk_size = bytes_per_pixel + 1;
                 GLubyte chunk[MAX_PIXEL_BYTES];
				 GLubyte* curr_chunk = *image_data;
				 for(int i = 0; i < header.width * header.height;)
				 {
					 /* read chunk (header+pixel) */
                     if(fread(chunk, 1, chunk_size, file) != chunk_size)
					 {
						 log_error("texture:img_load", "Unexpected end of file at chunk %d", i);
						 memory_free(*image_data);
						 *image_data = NULL;
						 return success;
					 }
					 i++;
					 copy_tga_pixel(&chunk[1], curr_chunk, bytes_per_pixel);
					 curr_chunk += bytes_per_pixel;
					 GLubyte chunk_identifier = chunk[0];
					 int count = chunk[0] & 0x7f;
					 if(chunk_identifier & 0x80) /* RLE chunk */
					 {
						 for(int j = 0; j < count; j++)
						 {
							 copy_tga_pixel(&chunk[1], curr_chunk, bytes_per_pixel);
							 curr_chunk += bytes_per_pixel;
							 i++;
						 }
					 }
					 else		/* Normal chunk */
					 {
						 for(int j = 0; j < count; j++)
						 {
							 if(fread(&chunk[1], 1, bytes_per_pixel, file) != bytes_per_pixel)
							 {
								 log_error("texture:img_load", "Unexpected end of file at pixel %d", i);
								 memory_free(*image_data);
								 *image_data = NULL;
								 return success;
							 }
							 copy_tga_pixel(&chunk[1], curr_chunk, bytes_per_pixel);
							 curr_chunk += bytes_per_pixel;
							 i++;
						 }
					 }
				 }
			 }
			 //debug_write_tga(&header, *image_data);
			 *height          = header.height;
			 *width           = header.width;
			 *fmt             = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;
			 *internal_format = *fmt;
			 success          = 1;
		 }
	 }
	 else
	 {
		 log_error("texture:load_img", "Could not read header");
		 success = 0;
	 }

	return success;
}

void texture_set_param(int index, int parameter, int value)
{
	struct Texture* texture = NULL;
	if(index > -1 && index < array_len(texture_list))
		texture = &texture_list[index];
	else
		return;

	GLint curr_texture = 0;
	GL_CHECK(glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_texture));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture->handle));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, parameter, value));
	if(curr_texture != 0)
		glBindTexture(GL_TEXTURE_2D, curr_texture);
}

void debug_write_tga(struct Tga_Header* header, GLubyte* image_data)
{
	/* Debug only, write the loaded image to file */
	FILE* fptr = NULL;
	if ((fptr = fopen("tga_debug.tga","w")) == NULL) {
		fprintf(stderr,"Failed to open outputfile\n");
		exit(-1);
	}
	if(header->datatypecode == 10) header->datatypecode = 2; /* Only uncompressed supported currently */
	fwrite(header, sizeof(struct Tga_Header), 1, fptr);
	for (int i = 0; i < header->height * header->width; i++)
	{
		putc(image_data[2], fptr);
		putc(image_data[1], fptr);
		putc(image_data[0], fptr);
		image_data += 3;
	}
	fclose(fptr);
}

void copy_tga_pixel(GLubyte* source, GLubyte* dest, size_t bytes_per_pixel)
{
	dest[0] = source[2];
	dest[1] = source[1];
	dest[2] = source[0];
	if(bytes_per_pixel == 4) dest[3] = source[3];
}

int texture_get_textureunit(int index)
{
	assert(index > -1 && index < array_len(texture_list));
	return texture_list[index].texture_unit;
}

int texture_get_texture_handle(int index)
{
	assert(index > -1 && index < array_len(texture_list));
	return texture_list[index].handle;
}

void texture_inc_refcount(int index)
{
	assert(index > -1 && index < array_len(texture_list));
	texture_list[index].ref_count++;
}

void texture_dec_refcount(int index)
{
	assert(index > -1 && index < array_len(texture_list));
	texture_list[index].ref_count--;
}

int texture_create(const char* name,
				   int   	   texture_unit,
				   int   	   width,
				   int   	   height,
				   int   	   format,
				   int   	   internal_format,
				   int   	   type,
				   const void* data)
{
	assert(texture_unit > -1 && texture_unit <= TU_SHADOWMAP4);
	int index   = -1;
	uint handle = 0;
	create_gl_texture(&handle, width, height, format, internal_format, type, data);
	struct Texture* new_tex = NULL;
	if(array_len(empty_indices) > 0)
	{
		index = *array_get_last(empty_indices, int);
		array_pop(empty_indices);
		new_tex = &texture_list[index];
	}
	else
	{
		new_tex = array_grow(texture_list, struct Texture);
		index   = array_len(texture_list) - 1;
	}
	new_tex->name            = name ? str_new(name) : str_new("texture_%d", index);
	new_tex->handle          = handle;
	new_tex->ref_count       = 1;
	new_tex->texture_unit    = texture_unit;
	new_tex->format          = format;
	new_tex->internal_format = internal_format;
	new_tex->type            = type;
	return index;
}

void create_gl_texture(uint*       out_handle,
					  int          width,
					  int          height,
					  int          format,
					  int          internal_format,
					  int          type,
					  const void*  data)
{
	GL_CHECK(glGenTextures(1, out_handle));
	glBindTexture(GL_TEXTURE_2D, *out_handle);
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data));
	glBindTexture(GL_TEXTURE_2D, 0);	
}

void texture_resize(int index, int width, int height, const void* data)
{
	assert(index > -1 && index < array_len(texture_list));
	width  -= (width % 2);
	height -= (height % 2);
	struct Texture* texture = &texture_list[index];

	GLint curr_texture = 0;
	GL_CHECK(glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_texture));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture->handle));
	glTexImage2D(GL_TEXTURE_2D,
				 0,
				 texture->internal_format,
				 width,
				 height,
				 0,
				 texture->format,
				 texture->type,
				 data);
	if(curr_texture != 0)
		glBindTexture(GL_TEXTURE_2D, curr_texture);
}

const char* texture_get_name(int index)
{
	assert(index > -1 && index < array_len(texture_list));
	return texture_list[index].name;
}
