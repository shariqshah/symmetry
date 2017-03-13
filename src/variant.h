#ifndef VARIANT_H
#define VARIANT_H

#include "linmath.h"

enum Variant_Type
{
	VT_NONE = 0,
	VT_INT,
	VT_FLOAT,
	VT_DOUBLE,
	VT_VEC2,
	VT_VEC3,
	VT_VEC4,
	VT_QUAT,
	VT_MAT3,
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
		vec2   val_vec2;
		vec3   val_vec3;
		vec4   val_vec4;
		quat   val_quat;
		void*  val_voidptr;
	};
};

void variant_init_empty(struct Variant* variant);
void variant_init_mat4(struct Variant* out_new_variant, const mat4* source);
void variant_free(struct Variant* variant);


#endif
