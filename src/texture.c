#include "texture.h"
#include "array.h"
#include "file_io.h"
#include "string_utils.h"
#include "log.h"
#include "num_types.h"
#include "renderer.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

struct Texture
{
	char* name;
	uint  handle;
	int   ref_count;
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

int  load_img(FILE* file, GLubyte** image_data, int* width, int* height, int* fmt, int* internal_fmt);
void texture_debug_write_tga(struct Tga_Header* header, GLubyte* image_data);
	
void texture_init(void)
{
	texture_list = array_new(struct Texture);
	empty_indices = array_new(int);
}

int texture_create_from_file(const char* filename)
{
	assert(filename);
	int index = -1;
	uint handle = 0;
	char* full_path = str_new("textures/");
	full_path = str_concat(full_path, filename);
	FILE* file = io_file_open(full_path, "rb");
	int img_load_success = -1;
	
	if(file)
	{
		/* Load texture here */
		int width, height, internal_fmt, fmt;
		GLubyte* img_data = NULL;
		width = height = internal_fmt = fmt = -1;
		img_load_success = load_img(file, &img_data, &width, &height, &fmt, &internal_fmt);

		if(img_load_success)
		{
			struct Texture* new_texture = NULL;
			if(array_len(empty_indices) > 0)
			{
				index = *array_get_last(empty_indices, int);
				array_pop(empty_indices);
				new_texture = &texture_list[index];
			}
			else
			{
				new_texture = array_grow(texture_list, struct Texture);
				index = array_len(texture_list) - 1;
			}
			assert(new_texture);
			
			log_message("\nWidth : %d\nHeight : %d\nFormat : %s",
						width, height, fmt == GL_RGB ? "RGB" : "RGBA");
			glGenTextures(1, &handle);
			if(new_texture->name) free(new_texture->name);
			new_texture->name = str_new(filename);
			new_texture->ref_count = 1;
			new_texture->handle = handle;
			glBindTexture(GL_TEXTURE_2D, handle);
			texture_param_set(index, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			texture_param_set(index, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			texture_param_set(index,GL_TEXTURE_WRAP_S,GL_REPEAT);
			texture_param_set(index,GL_TEXTURE_WRAP_T,GL_REPEAT);
			glTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, img_data);
			renderer_check_glerror("texture:create");
			glBindTexture(GL_TEXTURE_2D, 0);
			free(img_data);
		}
		fclose(file);
	}
	else
	{
		log_error("texture:create_from_file", "Could not open file %s", filename);
	}
	free(full_path);
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
				if(texture->name) free(texture->name);
				texture->ref_count = -1;
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
}

void texture_bind(int index, int texture_unit)
{
	assert(index > -1 && index < array_len(texture_list));
	//glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, texture_list[index].handle);
}

void texture_unbind(int texture_unit)
{
	//glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int load_img(FILE* file, GLubyte** image_data, int* width, int* height, int* fmt, int* internal_fmt)
{	
	int success = 0;
	struct Tga_Header header; 
	size_t items_read = fread(&header, sizeof(struct Tga_Header), 1, file);
	 
	 if(items_read == 1)
	 {
		 /* log_message("sizeof struct : %d", sizeof(struct Tga_Header)); */
		 /* log_message("%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", */
		 /* 			 header.idlength, */
		 /* 			 header.colourmaptype, */
		 /* 			 header.datatypecode, */
		 /* 			 header.colourmaporigin, */
		 /* 			 header.colourmaplength, */
		 /* 			 header.colourmapdepth, */
		 /* 			 header.x_origin, */
		 /* 			 header.y_origin, */
		 /* 			 header.width, */
		 /* 			 header.height, */
		 /* 			 header.bitsperpixel, */
		 /* 			 header.imagedescriptor); */
		 if(header.datatypecode == 0)
		 {
			 log_error("texture:load_img", "No image data in file");
			 success = 0;
		 }
		 else
		 {
			 /* only compressed and uncompressed true color image data supported yet */
			 if(header.datatypecode != 2 && header.datatypecode != 10)
			 {
				 log_error("texture:load_img", "Unsupported image data type");
				 success = 0;
				 return success;
			 }

			 if(header.bitsperpixel != 24 && header.bitsperpixel != 32)
			 {
				 log_error("texture:load_img",
						   "Unsupported bitsperpixel size(%d), only 24 and 32 supported", header.bitsperpixel);
				 success = 0;
				 return success;
			 }

			 if(header.width <= 0 || header.height <= 0)
			 {
				 log_error("texture:load_img", "Invalid width and height (%d:%d)", header.width, header.height);
				 success = 0;
				 return success;
			 }

			 size_t bytes_per_pixel = header.bitsperpixel / 8;
			 size_t image_size = (bytes_per_pixel * header.width * header.height);
			 *image_data = malloc(image_size);
			 if(!*image_data)
			 {
				 log_error("texture:load_img", "Out of memory");
				 success = 0;
				 return success;
			 }

			 /* skip over unnecessary things like colormap data */
			 int skipover = 0;
			 skipover += header.idlength;
			 skipover += header.colourmaptype * header.colourmaplength;
			 fseek(file, skipover, SEEK_CUR);

			 /* Start reading pixel by pixel */
			 GLubyte pixel[bytes_per_pixel];
			 GLubyte* curr_pixel = *image_data;
			 for(int i = 0; i < header.width * header.height; i++)
			 {
			 	 if(header.datatypecode == 2) /* uncompressed image data */
			 	 {
			 		 if(fread(&pixel, 1, bytes_per_pixel, file) != bytes_per_pixel)
			 		 {
			 			 success = 0;
			 			 log_error("texture:load_img", "Unexpected end of file at pixel %d", i);
			 			 free(image_data);
			 			 return success;
			 		 }

					 /* Swap BGR to RGB */
					 curr_pixel[0] = pixel[2];
					 curr_pixel[1] = pixel[1];
					 curr_pixel[2] = pixel[0];
					 if(bytes_per_pixel == 4) curr_pixel[3] = pixel[3];

			 		 curr_pixel += bytes_per_pixel;
			 	 }
			 	 else if(header.datatypecode == 10) /* compressed image data */
			 	 {
			 		 log_message("Not implemented yet!");
			 	 }
			 }

			 texture_debug_write_tga(&header, *image_data);
			 *height = header.height;
			 *width = header.width;
			 *fmt = bytes_per_pixel == 3 ? GL_RGB : GL_RGBA;
			 *internal_fmt = *fmt;
			 success = 1;
		 }
	 }
	 else
	 {
		 log_error("texture:load_img", "Could not read header");
		 success = 0;
	 }

	return success;
}

void texture_param_set(int index, int parameter, int value)
{
	struct Texture* texture = NULL;
	if(index > -1 && index < array_len(texture_list))
		texture = &texture_list[index];
	else
		return;

	GLint curr_texture = 0;
	glGetIntegerv(GL_TEXTURE_2D, &curr_texture);
	glBindTexture(GL_TEXTURE_2D, texture->handle);
	glTexParameteri(GL_TEXTURE_2D, parameter, value);
	renderer_check_glerror("texture:param_set");
	if(curr_texture != 0)
		glBindTexture(GL_TEXTURE_2D, curr_texture);
}

void texture_debug_write_tga(struct Tga_Header* header, GLubyte* image_data)
{
	/* Debug only, write the loaded image to file */
	FILE* fptr = NULL;
	if ((fptr = fopen("tga_debug.tga","w")) == NULL) {
		fprintf(stderr,"Failed to open outputfile\n");
		exit(-1);
	}
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
