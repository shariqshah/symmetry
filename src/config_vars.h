#ifndef CONFIG_VARS_H
#define CONFIG_VARS_H

struct Hashmap;

void            config_vars_init(void);
void            config_vars_cleanup(void);
int             config_vars_load(const char* filename);
int             config_vars_save(const char* filename);
struct Hashmap* config_vars_get(void);


#endif
