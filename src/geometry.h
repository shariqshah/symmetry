#ifndef geometry_H
#define geometry_H

#include "num_types.h"
#include "linmath.h"

struct Transform;

void geom_init(void);
int  geom_create_from_file(const char* name);
int  geom_find(const char* filename);
void geom_remove(int index);
void geom_cleanup(void);
void geom_render(int index);
void geom_render_in_frustum(int index, vec4* frustum, struct Transform* transform);
int  geom_create(const char* name,
				 vec3* vertices,
				 vec2* uvs,
				 vec3* normals,
				 uint* indices,
				 vec3* vertex_colors);

#endif
