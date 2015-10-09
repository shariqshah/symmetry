#ifndef texture_H
#define texture_H

void texture_init(void);
int  texture_create_from_file(const char* filename);
void texture_remove(int index);
int  texture_find(const char* name);
void texture_cleanup(void);
void texture_bind(int index, int texture_unit);
void texture_unbind(int texture_unit);
void texture_param_set(int index, int parameter, int value);

#endif
