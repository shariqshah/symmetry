#ifndef material_H
#define material_H

struct Model;

struct Uniform
{
	int   location;
	int   type;
	char* name;
};

struct Material_Param
{
	int   uniform_index;		/* Index of the corresponding uniform in the material's model_params */
	void* value;				/* Actual value of the uniform */
};

struct Material
{
	char* name;
	int   shader;
	int*  registered_models;
	int   active;
	struct Uniform* model_params; /* uniforms related to models */
	struct Uniform* pipeline_params; /* general uniforms like matrices etc */
};

struct Material* material_get_all_materials(void);
struct Material* material_find(const char* material_name);
struct Material* material_get(int index);
int  material_get_index(const char* material_name);
void material_init(void);
void material_cleanup(void);
int  material_register_model(struct Model* model, int model_index, const char* material_name);
void material_unregister_model(struct Model* model, int model_index);
void material_remove(int index);

#endif
