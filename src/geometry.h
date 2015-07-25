#ifndef geometry_H
#define geometry_H

void geom_init(void);
int  geom_ceate(const char* name);
int  geom_find(const char* filename);
void geom_remove(int index);
void geom_cleanup(void);

#endif
