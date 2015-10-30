#ifndef texture_H
#define texture_H

enum Texture_Unit
{
	TU_DIFFUSE = 0,
	TU_SHADOWMAP1,
	TU_SHADOWMAP2,
	TU_SHADOWMAP3,
	TU_SHADOWMAP4
};

void texture_init(void);
int  texture_create_from_file(const char* filename, int texture_unit);
void texture_remove(int index);
int  texture_find(const char* name);
void texture_cleanup(void);
void texture_bind(int index);
void texture_unbind(int index);
void texture_set_param(int index, int parameter, int value);
int  texture_get_textureunit(int index);
int  texture_get_texture_handle(int index);
void texture_inc_refcount(int index);
void texture_dec_refcount(int index);
int  texture_create(const char* name,
					int         texture_unit,
					int         width,
					int         height,
					int         format,
					int         int_fmt,
					int         type,
					void*       data);

#endif
