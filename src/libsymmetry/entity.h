#ifndef ENTITY_H
#define ENTITY_H

#include "../common/linmath.h"
#include "../common/num_types.h"
#include "../common/common.h"
#include "material.h"

#define MAX_ENTITY_NAME_LEN 128
#define MAX_SOUND_SOURCE_BUFFERS 5


struct Entity;
struct Material_Param;
struct Parser_Object;

typedef void (*Collision_CB)(struct Entity* this_entity, struct Entity* other_entity, Rigidbody, Rigidbody);

enum Entity_Type
{
	ET_NONE,
	ET_DEFAULT,
	ET_PLAYER,
	ET_ROOT,
	ET_CAMERA,
	ET_LIGHT,
	ET_STATIC_MODEL,
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

enum Camera_Type
{
	CT_EDITOR = 0,
	CT_GAME,
	CT_MAX
};

struct Transform
{
	vec3            position;
	vec3            scale;
	quat            rotation;
	mat4            trans_mat;
	bool            is_modified;
	bool            sync_physics;
	struct Entity*  parent;
	struct Entity** children;
};

struct Entity
{
	int              id;
	int              type;
	char             name[MAX_ENTITY_NAME_LEN];
	bool             marked_for_deletion;
	bool             active;
	bool             editor_selected;
	struct Transform transform;
};

struct Model
{
	int              geometry_index;
	struct Material* material;
	struct Variant   material_params[MMP_MAX];
};

struct Sound_Source
{
	struct Entity               base;
	int                         type;
	bool                        playing;
	bool                        loop;
    uint                        source_instance;
	float                       min_distance;
	float                       max_distance;
	float                       rolloff_factor;
	float                       volume;
	int                         attenuation_type;
	struct Sound_Source_Buffer* source_buffer; // Handle to the file from which the sound is loaded and played
};

struct Camera
{
	struct Entity base;
	mat4          proj_mat;
	mat4          view_mat;
	mat4          view_proj_mat;
	float         fov;
	float         aspect_ratio;
	float         nearz;
	float         farz;
	float         zoom;
	bool          ortho;
	int           fbo;
	int           render_tex;
	int           depth_tex;
	vec4          clear_color;
	vec4          frustum[6];
	bool          resizeable;
};

struct Light
{
	struct Entity base;
	float         outer_angle;
	float         inner_angle;
	float         falloff;
	float         intensity;
	vec3          color;
	bool          cast_shadow;
	bool          pcf_enabled;
	bool          valid;        
	int           type;
	int           radius; 
	int           shadow_map[4];
	float         depth_bias;
};

struct Collision
{
	Rigidbody       rigidbody;
	Collision_Shape collision_shape;
	Collision_CB    on_collision;
};

struct Static_Mesh
{
	struct Entity    base;
	struct Model     model;
	struct Collision collision;
};

struct Player
{
	struct Entity base;
};

void           entity_init(struct Entity* entity, const char* name, struct Entity* parent);
void           entity_reset(struct Entity* entity, int id);
bool           entity_save(struct Entity* entity, const char* filename, int directory_type);
bool           entity_load(const char* filename, int directory_type);
bool           entity_write(struct Entity* entity, struct Parser_Object* object);
struct Entity* entity_read(struct Parser_Object* object);
const char*    entity_type_name_get(struct Entity* entity);
void           entity_rigidbody_on_move(Rigidbody body);
void           entity_rigidbody_on_collision(Rigidbody body_A, Rigidbody body_B);
void           entity_rigidbody_set(struct Entity* entity, struct Collision* collision, Rigidbody body);
void           entity_collision_shape_set(struct Entity* entity, struct Collision* collision, Collision_Shape shape); // Only used for collision shapes like plane which can't have a rigidbody attached to collision shape

#endif
