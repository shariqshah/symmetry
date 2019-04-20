#ifndef CONFIG_VARS_H
#define CONFIG_VARS_H

#include "../common/num_types.h"

struct Hashmap;

void config_vars_init(struct Hashmap* cvars);
void config_vars_cleanup(struct Hashmap* cvars);
bool config_vars_load(struct Hashmap* cvars, const char* filename, int directory_type);
bool config_vars_save(struct Hashmap* cvars, const char* filename, int directory_types);

#endif
