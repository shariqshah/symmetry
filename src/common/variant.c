#include "variant.h"
#include "log.h"
#include "string_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void variant_init_empty(struct Variant* variant)
{
	variant->type        = VT_NONE;
	variant->val_voidptr = NULL;
	variant->val_mat4    = NULL;
	variant->val_str     = NULL;
}

void variant_assign_float(struct Variant* variant, const float value)
{
	if(variant->type != VT_FLOAT) variant_free(variant);
	variant->type      = VT_FLOAT;
	variant->val_float = value;
}

void variant_assign_int(struct Variant* variant, const int value)
{
	if(variant->type != VT_INT) variant_free(variant);
	variant->type    = VT_INT;
	variant->val_int = value;
}

void variant_assign_uint(struct Variant* variant, const uint value)
{
    if(variant->type != VT_UINT) variant_free(variant);
    variant->type     = VT_UINT;
    variant->val_uint = value;
}

void variant_assign_double(struct Variant* variant, const double value)
{
	if(variant->type != VT_DOUBLE) variant_free(variant);
	variant->type       = VT_DOUBLE;
	variant->val_double = value;
}

void variant_assign_bool(struct Variant* variant, const bool value)
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

void variant_assign_vec2f(struct Variant* variant, const float x, const float y)
{
	if(variant->type != VT_VEC2) variant_free(variant);
	variant->type     = VT_VEC2;
	vec2_fill(&variant->val_vec2, x, y);
}

void variant_assign_vec3f(struct Variant* variant, const float x, const float y, const float z)
{
	if(variant->type != VT_VEC3) variant_free(variant);
	variant->type     = VT_VEC3;
	vec3_fill(&variant->val_vec3, x, y, z);
}

void variant_assign_vec4f(struct Variant* variant, const float x, const float y, const float z, const float w)
{
	if(variant->type != VT_VEC4) variant_free(variant);
	variant->type     = VT_VEC4;
	vec4_fill(&variant->val_vec4, x, y, z, w);
}

void variant_assign_quatf(struct Variant* variant, const float x, const float y, const float z, const float w)
{
	if(variant->type != VT_QUAT) variant_free(variant);
	variant->type     = VT_QUAT;
	quat_fill(&variant->val_quat, x, y, z, w);
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
	case VT_BOOL:   variant_assign_bool(to, from->val_bool);     break;
	case VT_INT:    variant_assign_int(to, from->val_int);       break;
    case VT_UINT:   variant_assign_uint(to, from->val_uint);     break;
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

void variant_to_str(const struct Variant* variant, char* str, int len)
{
	if(!variant) return;
	switch(variant->type)
	{
	case VT_BOOL:   snprintf(str, len, "%s", variant->val_bool ? "true" : "false"); break;
	case VT_INT:    snprintf(str, len, "%d", variant->val_int); break;
    case VT_UINT:   snprintf(str, len, "%d", variant->val_uint); break;
	case VT_FLOAT:  snprintf(str, len, "%.4f", variant->val_float); break;
	case VT_DOUBLE: snprintf(str, len, "%.4f", variant->val_double); break;
	case VT_VEC2:   snprintf(str, len, "%.3f %.3f", variant->val_vec2.x, variant->val_vec2.y); break;
	case VT_VEC3:   snprintf(str, len, "%.3f %.3f %.3f", variant->val_vec3.x, variant->val_vec3.y, variant->val_vec3.z); break;
	case VT_VEC4:   snprintf(str, len, "%.3f %.3f %.3f %.3f", variant->val_vec4.x, variant->val_vec4.y, variant->val_vec4.z, variant->val_vec4.w); break;
	case VT_QUAT:   snprintf(str, len, "%.3f %.3f %.3f %.3f", variant->val_quat.x, variant->val_quat.y, variant->val_quat.z, variant->val_quat.w); break;
	case VT_STR:    snprintf(str, len, "%s", variant->val_str); break;
	case VT_NONE:   snprintf(str, len, "%s", "NONE"); break;
	default:        snprintf(str, len, "Unsupported Variant type"); break;
	}
}

void variant_from_str(struct Variant* variant, const char* str, int variant_type)
{
	assert(variant_type > -1 && variant_type < VT_NUM_TYPES);
	static char str_val[MAX_VARIANT_STR_LEN] = {'\0'};
    switch(variant_type)
	{
	case VT_BOOL:
	{
		bool boolean = false;
		memset(str_val, '\0', MAX_VARIANT_STR_LEN);
		if(sscanf(str, "%1024s", str_val) == 1)
		{
			if(strncmp("true", str_val, 5) == 0)
				boolean = true;
			else if(strncmp("false", str_val, 5) == 0)
				boolean = false;

			variant_assign_bool(variant, boolean);
		}
	}
	break;
	case VT_INT:
	{
		int int_val = -1;
		if(sscanf(str, "%d", &int_val) == 1)
			variant_assign_int(variant, int_val);
	}
	break;
    case VT_UINT:
    {
        uint uint_val = 0;
        if(sscanf(str, "%d", &uint_val) == 1)
            variant_assign_uint(variant, uint_val);
    }
    break;
	case VT_FLOAT:
	{
		float float_val = -1;
		if(sscanf(str, "%f", &float_val) == 1)
			variant_assign_float(variant, float_val);
	}
	break;
	case VT_DOUBLE:
	{
		double double_val = -1;
		if(sscanf(str, "%lf", &double_val) == 1)
			variant_assign_double(variant, double_val);
	}
	break;
	case VT_STR:
	{
		memset(str_val, '\0', MAX_VARIANT_STR_LEN);
		if(sscanf(str, "%1024s", str_val) == 1)
			variant_assign_str(variant, str_val);
	}
	break;
	case VT_VEC2:
	{
		vec2 vec2_val = {.x = 0.f, .y = 0.f};
		if(sscanf(str, "%f %f", &vec2_val.x, &vec2_val.y) == 2)
			variant_assign_vec2(variant, &vec2_val);
	}
	break;
	case VT_VEC3:
	{
		vec3 vec3_val = {.x = 0.f, .y = 0.f, .z = 0.f};
		if(sscanf(str, "%f %f %f", &vec3_val.x, &vec3_val.y, &vec3_val.z) == 3)
			variant_assign_vec3(variant, &vec3_val);
	}
	break;
	case VT_VEC4:
	{
		vec4 vec4_val = {.x = 0.f, .y = 0.f, .z = 0.f, .w = 0.f};
		if(sscanf(str, "%f %f %f %f", &vec4_val.x, &vec4_val.y, &vec4_val.z, &vec4_val.w) == 4)
			variant_assign_vec4(variant, &vec4_val);
	}
	break;
	case VT_QUAT:
	{
		quat quat_val = {.x = 0.f, .y = 0.f, .z = 0.f, .w = 0.f};
		if(sscanf(str, "%f %f %f %f", &quat_val.x, &quat_val.y, &quat_val.z, &quat_val.w) == 4)
			variant_assign_quat(variant, &quat_val);
	}
	break;
	default: /* Other types not supported, quietly return */ break;
	}
}

void variant_copy_out(void* to, const struct Variant* from)
{
	switch(from->type)
	{
	case VT_BOOL:   *(bool*)to   = from->val_bool;                     break;
	case VT_INT:    *(int*)to    = from->val_int;                      break;
    case VT_UINT:   *(uint*)to   = from->val_uint;                     break;
	case VT_FLOAT:  *(float*)to  = from->val_float;                    break;
	case VT_DOUBLE: *(double*)to = from->val_double;                   break;
	case VT_VEC2:   vec2_assign((vec2*)to, &from->val_vec2);           break;
	case VT_VEC3:   vec3_assign((vec3*)to, &from->val_vec3);           break;
	case VT_VEC4:   vec4_assign((vec4*)to, &from->val_vec4);           break;
	case VT_QUAT:   quat_assign((quat*)to, &from->val_quat);           break;
	case VT_MAT4:   mat4_assign((mat4*)to, from->val_mat4);            break;
	case VT_STR:    strncpy(to, from->val_str, strlen(from->val_str)); break;
	default: /* Nothing to be done for the rest*/
		break;
	}
}
