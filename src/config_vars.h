#ifndef CONFIG_VARS_H
#define CONFIG_VARS_H

#include "num_types.h"

struct Hashmap;

void            config_vars_init(void);
void            config_vars_cleanup(void);
bool            config_vars_load(const char* filename, int directory_type);
bool            config_vars_save(const char* filename, int directory_types);
struct Hashmap* config_vars_get(void);


#endif
