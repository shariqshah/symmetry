#ifndef LIGHT_H
#define LIGHT_H

#define MAX_SHADOWMAPS 4

struct Light;

void light_init(void);
void light_cleanup(void);
void light_destroy(struct Light* light, int entity_id);
void light_create(struct Light* light, int entity_id, int light_type);
int* light_get_valid_indices(int* out_count);

#endif
