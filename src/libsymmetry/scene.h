#ifndef SCENE_H
#define SCENE_H

#include "../common/num_types.h"

struct Entity;

void           scene_init(void);
void           scene_remove(struct Entity* entity);
void           scene_reset_parent(struct Entity* entity, struct Entity* new_parent);
void           scene_cleanup(void);
struct Entity* scene_add_new(const char* name, const int type); /* Add as child of Root */
struct Entity* scene_add_as_child(const char* name, const int type, int parent);
struct Entity* scene_find(const char* name);
struct Entity* scene_get_root(void);
void           scene_root_set(struct Entity* entity);
struct Entity* scene_get_child_by_name(struct Entity* parent, const char* name);
struct Entity* scene_get_parent(struct Entity* entity);
bool           scene_load(const char* filename, int directory_type);
bool           scene_save(const char* filename, int directory_type);


#endif
