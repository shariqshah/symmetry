#ifndef ENTITY_H
#define ENTITY_H

#include "../common/linmath.h"
#include "../common/num_types.h"
#include "../common/common.h"

#define MAX_ENTITY_NAME_LEN 128
#define MAX_SOUND_SOURCE_BUFFERS 5

struct Material_Param;

enum Entity_Type
{
	ET_NONE,
	ET_PLAYER,
	ET_ROOT,
	ET_CAMERA,
	ET_LIGHT,
	ET_STATIC_MESH,
	ET_SOUND_SOURCE,
	ET_MAX
};

enum LightType
{
	LT_SPOT  = 0,
	LT_DIR,
	LT_POINT,
	LT_INVALID,
	LT_MAX
};

struct Transform
{
	vec3 position;
	vec3 scale;
	quat rotation;
	mat4 trans_mat;
	int  parent;
	int* children;
	bool is_modified;
};

struct Model
{
	int                    geometry_index;
	int                    material;
	struct Material_Param* material_params;
};

struct Sound_Source
{
	bool active;
	bool relative;
    uint source_handle;
    uint buffer_handles[MAX_SOUND_SOURCE_BUFFERS];
    uint num_attached_buffers;
};

struct Camera
{
	mat4  proj_mat;
	mat4  view_mat;
	mat4  view_proj_mat;
	float fov;
	float aspect_ratio;
	float nearz;
	float farz;
	bool  ortho;
	int   fbo;
	int   render_tex;
	int   depth_tex;
	vec4  clear_color;
	vec4  frustum[6];
	bool  resizeable;
};

struct Light
{
	float outer_angle;
	float inner_angle;
	float falloff;
	float intensity;
	vec3  color;
	bool  cast_shadow;
	bool  pcf_enabled;
	bool  valid;        
	int   type;
	int   radius; 
	int   shadow_map[4];
	float depth_bias;
};

struct Entity
{
	int              id;
	int              type;
	char             name[MAX_ENTITY_NAME_LEN];
	bool             is_listener; /* TODO: Replace all booleans with flags */
	bool             marked_for_deletion;
	bool             renderable;
	int              editor_selected;
	struct Transform transform;

	union
	{
		struct
		{
			struct Model model;
			int          health;
		} Player;

		struct Model        model;
		struct Camera       camera;
		struct Light        light;
		struct Sound_Source sound_source;
	};
};

void           entity_init(void);
void           entity_cleanup(void);
void           entity_remove(int index);
void           entity_post_update(void);
struct Entity* entity_create(const char* name, const int type, int parent_id);
struct Entity* entity_get(int index);
struct Entity* entity_find(const char* name);
struct Entity* entity_get_all(void);
struct Entity* entity_get_parent(int node);
bool           entity_save(struct Entity* entity, const char* filename, int directory_type);
struct Entity* entity_load(const char* filename, int directory_type);
bool           entity_write(struct Entity* entity, FILE* file);
struct Entity* entity_read(FILE* file);
const char*    entity_type_name_get(struct Entity* entity);


#endif