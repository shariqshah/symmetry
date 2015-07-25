#ifndef _transform_H
#define _transform_H

#include "linmath.h"

struct Transform
{
	vec3 position;
	vec3 scale;
	quat rotation;
};

void transform_initialize(void);
void transform_cleanup(void);
int  transform_create(int node);

#endif
