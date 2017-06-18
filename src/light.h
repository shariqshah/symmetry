#ifndef LIGHT_H
#define LIGHT_H

#define MAX_SHADOWMAPS 4

struct Entity;

void light_init(void);
void light_cleanup(void);
void light_destroy(struct Entity* entity);
void light_create(struct Entity* entity, int light_type);
void light_add(struct Entity* entity);
int* light_get_valid_indices(int* out_count);

#endif
