#include "config_vars.h"
#include "string_utils.h"
#include "variant.h"
#include "hashmap.h"

#include <stdlib.h>

static struct Hashmap* cvars = NULL;

void config_vars_init(void)
{
	cvars = hashmap_new();
}

void config_vars_cleanup(void)
{
	hashmap_free(cvars);
}

struct Hashmap* config_vars_get(void)
{
	return cvars;
}

int config_vars_load(const char* filename)
{
	
}

void config_vars_save(const char* filename)
{
	
}
