#include "variant.h"
#include "log.h"
#include <stdlib.h>

void variant_init_empty(struct Variant* variant)
{
	variant->type = VT_NONE;
	variant->val_voidptr = NULL;
}

void variant_assign_mat4(struct Variant* variant, const mat4* source)
{
	if(variant->type != VT_MAT4) variant_free(variant);
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

void variant_assign_float(struct Variant* variant, float value)
{
	if(variant->type != VT_FLOAT) variant_free(variant);
	variant->type      = VT_FLOAT;
	variant->val_float = value;
}

void variant_assign_int(struct Variant* variant, int value)
{
	if(variant->type != VT_INT) variant_free(variant);
	variant->type    = VT_INT;
	variant->val_int = value;
}

void variant_assign_double(struct Variant* variant, double value)
{
	if(variant->type != VT_DOUBLE) variant_free(variant);
	variant->type       = VT_DOUBLE;
	variant->val_double = value;
}

void variant_assign_vec2(struct Variant* variant, vec2* value)
{
	if(variant->type != VT_VEC2) variant_free(variant);
	variant->type     = VT_VEC2;
	vec2_assign(&variant->val_vec2, value);
}

void variant_assign_vec3(struct Variant* variant, vec3* value)
{
	if(variant->type != VT_VEC3) variant_free(variant);
	variant->type     = VT_VEC3;
	vec3_assign(&variant->val_vec3, value);
}

void variant_assign_vec4(struct Variant* variant, vec4* value)
{
	if(variant->type != VT_VEC4) variant_free(variant);
	variant->type     = VT_VEC4;
	vec4_assign(&variant->val_vec4, value);
}

void variant_assign_quat(struct Variant* variant, quat* value)
{
	if(variant->type != VT_QUAT) variant_free(variant);
	variant->type     = VT_QUAT;
	quat_assign(&variant->val_quat, value);
}




void variant_free_mat4(struct Variant* variant)
{
	if(variant->type == VT_MAT4)
	{
		if(variant->val_voidptr)
		{
			free(variant->val_voidptr);
			variant->val_voidptr = NULL;
		}
	}
	else
	{
		log_error("variant:free_mat4", "Cannot free, variant is not of MAT4 type");
	}
}

void variant_free(struct Variant* variant)
{
	if(variant->type == VT_MAT4)
		variant_free_mat4(variant);
}
