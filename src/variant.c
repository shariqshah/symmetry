#include "variant.h"
#include "log.h"
#include <stdlib.h>

void variant_init_empty(struct Variant* variant)
{
	variant->type = VT_NONE;
	variant->val_voidptr = NULL;
}

void variant_init_mat4(struct Variant* variant, const mat4* source)
{
	if(variant->type == VT_MAT4)
	{
		if(!variant->val_voidptr)
		{
			variant->val_voidptr = malloc(sizeof(mat4));
			if(!variant->val_voidptr)
			{
				log_error("variant_init_mat4", "Out of memory");
				return;
			}
		}
		mat4_assign(variant->val_voidptr, source);
	}
}

void variant_free_mat4(struct Variant* variant)
{
	if(variant->type == VT_MAT4)
	{
		if(variant->val_voidptr) free(variant->val_voidptr);
	}
	else
	{
		log_error("variant:free_mat4", "Cannot free, variant is not of MAT4 type");
	}
}
