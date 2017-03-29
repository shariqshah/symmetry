#include "variant.h"
#include "log.h"
#include "string_utils.h"

#include <stdlib.h>

void variant_init_empty(struct Variant* variant)
{
	variant->type        = VT_NONE;
	variant->val_voidptr = NULL;
	variant->val_mat4    = NULL;
	variant->val_str     = NULL;
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

void variant_assign_bool(struct Variant* variant, int value)
{
	if(variant->type != VT_BOOL) variant_free(variant);
	variant->type     = VT_BOOL;
	variant->val_bool = value;
}

void variant_assign_str(struct Variant* variant, const char* value)
{
	if(variant->type != VT_STR) variant_free(variant);
	variant->type    = VT_STR;
	variant->val_str = str_new(value);
}

void variant_assign_vec2(struct Variant* variant, const vec2* value)
{
	if(variant->type != VT_VEC2) variant_free(variant);
	variant->type     = VT_VEC2;
	vec2_assign(&variant->val_vec2, value);
}

void variant_assign_vec3(struct Variant* variant, const vec3* value)
{
	if(variant->type != VT_VEC3) variant_free(variant);
	variant->type     = VT_VEC3;
	vec3_assign(&variant->val_vec3, value);
}

void variant_assign_vec4(struct Variant* variant, const vec4* value)
{
	if(variant->type != VT_VEC4) variant_free(variant);
	variant->type     = VT_VEC4;
	vec4_assign(&variant->val_vec4, value);
}

void variant_assign_quat(struct Variant* variant, const quat* value)
{
	if(variant->type != VT_QUAT) variant_free(variant);
	variant->type     = VT_QUAT;
	quat_assign(&variant->val_quat, value);
}

void variant_assign_mat4(struct Variant* variant, const mat4* source)
{
	if(variant->type != VT_MAT4) variant_free(variant);
	variant->val_mat4 = malloc(sizeof(mat4));
	if(!variant->val_mat4)
	{
		log_error("variant_init_mat4", "Out of memory");
		return;
	}
	mat4_assign(variant->val_voidptr, source);
}

void variant_assign_ptr(struct Variant* variant, void* source)
{
	if(variant->type != VT_VOID_PTR) variant_free(variant);
	variant->type        = VT_VOID_PTR;
	variant->val_voidptr = source;
}

void variant_free(struct Variant* variant)
{
	switch(variant->type)
	{
	case VT_MAT4:
		if(variant->val_mat4)
		{
			free(variant->val_mat4);
			variant->val_mat4 = NULL;
		}
		break;
	case VT_STR:
		if(variant->val_str)
		{
			free(variant->val_str);
			variant->val_str = NULL;
		}
		break;
	case VT_VOID_PTR:
		variant->val_voidptr = NULL;
		break;
	default: /* Nothing to be done for the rest*/
		break;
	};
	variant->type = VT_NONE;
}

void variant_copy(struct Variant* to, const struct Variant* from)
{
	to->type = from->type;
	switch(from->type)
	{
	case VT_BOOL:   variant_assign_bool(to, from->val_int);      break;
	case VT_INT:    variant_assign_int(to, from->val_bool);      break;
	case VT_FLOAT:  variant_assign_float(to, from->val_float);   break;
	case VT_DOUBLE: variant_assign_double(to, from->val_double); break;
	case VT_VEC2:   variant_assign_vec2(to, &from->val_vec2);    break;
	case VT_VEC3:   variant_assign_vec3(to, &from->val_vec3);    break;
	case VT_VEC4:   variant_assign_vec4(to, &from->val_vec4);    break;
	case VT_QUAT:   variant_assign_quat(to, &from->val_quat);    break;
	case VT_MAT4:   variant_assign_mat4(to, from->val_mat4);     break;
	case VT_STR:    variant_assign_str(to, from->val_str);       break;
	default: /* Nothing to be done for the rest*/
		break;
	}
}
