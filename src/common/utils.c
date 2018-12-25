#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define BUFF_SIZE 128
static char str_buff[BUFF_SIZE];

const char* tostr_vec3(vec3* v)
{
	memset(str_buff, '\0', BUFF_SIZE);
	snprintf(str_buff, BUFF_SIZE, "(%.3f, %.3f, %.3f)", v->x, v->y, v->z);
	return str_buff;
}

const char* tostr_vec4(vec4* v)
{
	memset(str_buff, '\0', BUFF_SIZE);
	snprintf(str_buff, BUFF_SIZE, "(%.3f, %.3f, %.3f, %.3f)", v->x, v->y, v->z, v->w);
	return str_buff;
}

const char* tostr_quat(quat* q)
{
	memset(str_buff, '\0', BUFF_SIZE);
	snprintf(str_buff, BUFF_SIZE, "(%.3f, %.3f, %.3f, %.3f)", q->x, q->y, q->z, q->w);
	return str_buff;
}

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

int clamp(int num, int upper, int lower)
{
    return min(upper, max(num, lower));
}
