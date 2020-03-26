#ifndef ENTITY_H
#define ENTITY_H

#include "../common/linmath.h"
#include "../common/num_types.h"
#include "../system/sound.h"
#include "bounding_volumes.h"
#include "material.h"
#include "../common/limits.h"


struct Entity;
struct Material_Param;
struct Parser_Object;

typedef void (*Trigger_Func)(struct Trigger* trigger);

enum Entity_Type
{
    ET_NONE,
    ET_DEFAULT,
    ET_PLAYER,
    ET_ROOT,
    ET_CAMERA,
    ET_LIGHT,
    ET_STATIC_MESH,
    ET_SOUND_SOURCE,
	ET_ENEMY,
	ET_TRIGGER,
	ET_DOOR,
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
    CAM_EDITOR = 0,
    CAM_GAME,
    CAM_MAX
};

enum Enemy_Type
{
	ENEMY_TURRET = 0,
	ENEMY_MAX
};

enum Entity_Flags
{
	EF_NONE                           = 0,
	EF_ACTIVE                         = 1 << 0,
	EF_SELECTED_IN_EDITOR             = 1 << 1,
	EF_MARKED_FOR_DELETION            = 1 << 2,
	EF_TRANSIENT                      = 1 << 3, // Do not save the entity when saving the scene. The entity will still be saved if it is individually saved to file
	EF_HIDE_IN_EDITOR_SCENE_HIERARCHY = 1 << 4,
	EF_SKIP_RENDER                    = 1 << 5,
	EF_IGNORE_RAYCAST                 = 1 << 6
};

enum Entity_Ray_Mask
{
	ERM_NONE         = 0,
	ERM_DEFAULT      = 1 << 0,
	ERM_PLAYER       = 1 << 1,
	ERM_CAMERA       = 1 << 2,
	ERM_LIGHT        = 1 << 3,
	ERM_STATIC_MESH  = 1 << 4,
	ERM_SOUND_SOURCE = 1 << 5,
	ERM_ENEMY        = 1 << 6,
	ERM_TRIGGER      = 1 << 7,
	ERM_ALL          = ERM_DEFAULT | ERM_PLAYER | ERM_CAMERA | ERM_LIGHT | ERM_STATIC_MESH | ERM_SOUND_SOURCE | ERM_ENEMY | ERM_TRIGGER
};

enum Trigger_Mask
{
	TRIGM_PLAYER = 1 << 0,
	TRIGM_ENEMY  = 1 << 1,
	TRIGM_ALL    = TRIGM_PLAYER | TRIGM_ENEMY
};

enum Door_Key_Mask
{
	DOOR_KEY_MASK_NONE  = 0,
	DOOR_KEY_MASK_RED   = 1 << 0,
	DOOR_KEY_MASK_GREEN = 1 << 1, 
	DOOR_KEY_MASK_BLUE  = 1 << 2,
	DOOR_KEY_MASK_ALL   = DOOR_KEY_MASK_RED | DOOR_KEY_MASK_GREEN | DOOR_KEY_MASK_BLUE
};

enum Trigger_Type
{
	TRIG_TOGGLE = 0,  // Toggled on once and fires event then wont fire event until it is deactivated and activated again
	TRIG_CONTINUOUS,  // Continuously fire events while the trigger is active
	TRIG_ONE_SHOT,    // Fire event once when triggered and then get deleted
	TRIG_MAX
};

struct Transform
{
    vec3            position;
    vec3            scale;
    quat            rotation;
    mat4            trans_mat;
    bool            is_modified;
    struct Entity*  parent;
    struct Entity** children;
};

struct Entity
{
    int                 id;
    int                 type;
	int                 archetype_index;
	uchar               flags;
    char                name[MAX_ENTITY_NAME_LEN];
	struct Bounding_Box bounding_box;
	struct Bounding_Box derived_bounding_box;
    struct Transform    transform;
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


struct Static_Mesh
{
    struct Entity    base;
    struct Model     model;
};

struct Player
{
    struct Entity        base;
    struct Static_Mesh*  mesh;
    struct Camera*       camera;
	struct Sound_Source* weapon_sound;
	struct Sound_Source* footstep_sound;
	struct Sound_Source* grunt_sound;
	int                  health;
	int                  key_mask;
    float				 move_speed;
    float				 move_speed_multiplier;
    float				 turn_speed;
	float				 jump_speed;
	float				 gravity;
	float				 min_downward_distance;
	float				 min_forward_distance;
	bool				 grounded;
	bool                 can_jump;
};

struct Enemy
{
	struct Entity        base;
	int                  type;
	int                  health;
	int                  damage;
	int                  current_state;
	struct Static_Mesh*  mesh;
	struct Sound_Source* weapon_sound;
	struct Sound_Source* ambient_sound;
	union
	{
		struct
		{
			float turn_speed_default;
			float turn_speed_when_targetting;
			float turn_speed_current;
			float max_yaw;
			float target_yaw;
			bool  yaw_direction_positive;
			bool  pulsate;
			bool  scan;
			float pulsate_speed_scale;
			float pulsate_height;
			float attack_cooldown;
			float time_elapsed_since_attack;
			float time_elapsed_since_alert;
			float alert_cooldown;
			float vision_range;
			vec4  color_default;
			vec4  color_alert;
			vec4  color_attack;
		}Turret;
	};
};

struct Trigger
{
	struct Entity base;
	bool          triggered;
	int           type;
	int           count;
	int           trigger_mask;
};

struct Door
{
	struct Entity        base;
	int                  mask;
	int                  state;
	float                speed;
	float                open_position;
	float                close_position;
	struct Static_Mesh*  mesh;
	struct Sound_Source* sound;
	struct Trigger*      trigger;
};

void           entity_init(struct Entity* entity, const char* name, struct Entity* parent);
void           entity_reset(struct Entity* entity, int id);
bool           entity_save(struct Entity* entity, const char* filename, int directory_type);
struct Entity* entity_load(const char* filename, int directory_type, bool send_on_scene_load_event);
bool           entity_write(struct Entity* entity, struct Parser_Object* object, bool write_transform);
struct Entity* entity_read(struct Parser_Object* object, struct Entity* parent_entity);
const char*    entity_type_name_get(struct Entity* entity);
int            entity_get_num_children_of_type(struct Entity* entity, int type, struct Entity** in_children, int max_children);
void           entity_rename(struct Entity* entity, const char* new_name);
void           entity_update_derived_bounding_box(struct Entity* entity);
void           entity_bounding_box_reset(struct Entity* entity, bool update_derived);

#endif
