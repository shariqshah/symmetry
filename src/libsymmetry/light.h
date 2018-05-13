#ifndef LIGHT_H
#define LIGHT_H

#define MAX_SHADOWMAPS 4

void light_init(struct Light* light, int light_type);
void light_reset(struct Light* light);

#endif
