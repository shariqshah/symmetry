#ifndef MATERIAL_H
#define MATERIAL_H

#include "../common/linmath.h"
#include "../common/num_types.h"

struct Entity;

struct Uniform
{
	int   location;
	char* name;
	int   type;
	union /* Default values */
	{
		vec2  d_vec2;
		vec3  d_vec3;
		vec4  d_vec4;
		int   d_int;
		float d_float;
	};
};

struct Material_Param
{
	int   uniform_index;		/* Index of the corresponding uniform in the material's model_params */
	void* value;				/* Actual value of the uniform */
};

struct Material
{
	char*           name;
	int             shader;
	int*            registered_models;
	bool            active;
	bool            lit;					  /* If material uses light information */
	struct Uniform* model_params; /* uniforms related to models */
	struct Uniform* pipeline_params; /* general uniforms like matrices etc */
};

struct Material* material_get_all_materials(void);
struct Material* material_find(const char* material_name);
struct Material* material_get(int index);
int 			 material_get_index(const char* material_name);
void			 material_init(void);
void			 material_cleanup(void);
bool 			 material_register_model(struct Entity* entity, const char* material_name);
void			 material_unregister_model(struct Entity* entity);
void			 material_remove(int index);

#endif
