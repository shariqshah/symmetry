#ifndef scene_H
#define scene_H

struct Entity;

void           scene_init(void);
void           scene_remove(struct Entity* entity);
void           scene_reset_parent(struct Entity* entity, struct Entity* new_parent);
void           scene_cleanup(void);
struct Entity* scene_add_new(const char* name, const char* tag); /* Add as child of Root */
struct Entity* scene_add_as_child(const char* name, const char* tag, struct Entity* parent);
struct Entity* scene_find(const char* name);
struct Entity* scene_get_root(void);
struct Entity* scene_get_child_by_name(struct Entity* parent, const char* name);

#endif
