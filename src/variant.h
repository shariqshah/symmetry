#ifndef VARIANT_H
#define VARIANT_H

#include "linmath.h"

enum Variant_Type
{
	VT_NONE = 0,
	VT_INT,
	VT_FLOAT,
	VT_DOUBLE,
	VT_BOOL,
	VT_STR,
	VT_VEC2,
	VT_VEC3,
	VT_VEC4,
	VT_QUAT,
	VT_MAT4,
	VT_VOID_PTR,
	VT_NUM_TYPES
};

struct Variant
{
	int type;
	union
	{
		int    val_int;
		float  val_float;
		double val_double;
		char*  val_str;
		vec2   val_vec2;
		vec3   val_vec3;
		vec4   val_vec4;
		quat   val_quat;
		void*  val_voidptr;
	};
};

void variant_init_empty(struct Variant* variant);
void variant_assign_mat4(struct Variant* variant, const mat4* source);
void variant_assign_float(struct Variant* variant, float value);
void variant_assign_int(struct Variant* variant, int value);
void variant_assign_double(struct Variant* variant, double value);
void variant_assign_bool(struct Variant* variant, int value);
void variant_assign_str(struct Variant* variant, const char* value);
void variant_assign_vec2(struct Variant* variant, vec2* value);
void variant_assign_vec3(struct Variant* variant, vec3* value);
void variant_assign_vec4(struct Variant* variant, vec4* value);
void variant_assign_quat(struct Variant* variant, quat* value);
void variant_free(struct Variant* variant);

#endif
