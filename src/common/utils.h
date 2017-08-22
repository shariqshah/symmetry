#ifndef UTILS_H
#define UTILS_H

#include "linmath.h"

const char* tostr_vec3(vec3* v);
const char* tostr_vec4(vec4* v);
const char* tostr_quat(quat* q);

// Math related functions
int min(int a, int b);
int max(int a, int b);
int clamp(int num, int min, int max);

#endif
