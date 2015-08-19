#ifndef geometry_H
#define geometry_H

void geom_init(void);
int  geom_create(const char* name);
int  geom_find(const char* filename);
void geom_remove(int index);
void geom_cleanup(void);
void geom_render(int index);

#endif
