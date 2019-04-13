#ifndef RENDERER_H
#define RENDERER_H

#include "../common/linmath.h"
#include "../common/num_types.h"
#include "material.h"

struct Sprite_Batch;
struct Scene;

enum Fog_Mode
{
    FM_NONE             = 0,
    FM_LINEAR           = 1,
    FM_EXPONENTIAL      = 2,
    FM_EXPONENTIAL_SQRD = 3
};

struct Fog
{
    int   mode;
    float density;
    float start_dist;
    float max_dist;
    vec3  color;
};


struct Render_Settings
{
    struct Fog fog;
    vec3       ambient_light;
    bool       debug_draw_enabled;
    vec4       debug_draw_color;
    int        debug_draw_mode;
    bool       debug_draw_physics;
};

struct Renderer
{
    int def_fbo;
    int def_albedo_tex;
    int def_depth_tex;
    int quad_geo;
    int composition_shader;
    int debug_shader;
    int num_culled , num_rendered , num_indices;
    int num_culled_slot, num_rendered_slot, num_indices_slot;
    struct Sprite_Batch* sprite_batch;
    struct Render_Settings settings;
    struct Material materials[MAT_MAX];
};

void renderer_init(struct Renderer* renderer);
void renderer_draw(struct Renderer* renderer, struct Scene* scene);
void renderer_cleanup(struct Renderer* renderer);
void renderer_clearcolor_set(float r, float g, float b, float a);
void renderer_debug_draw_enabled(struct Renderer* renderer, bool enabled);

#endif
