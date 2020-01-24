#include "linmath.h"

#include <math.h>
#include <string.h>


const vec3 UNIT_X = { 1, 0, 0 };
const vec3 UNIT_Y = { 0, 1, 0 };
const vec3 UNIT_Z = { 0, 0, 1 };

const vec3 UNIT_X_NEG = { -1,  0,  0 };
const vec3 UNIT_Y_NEG = { 0, -1,  0 };
const vec3 UNIT_Z_NEG = { 0,  0, -1 };


void vec2_fill(vec2* res, float x, float y)
{
	res->x = x;
	res->y = y;
}

void vec2_add(vec2* res, vec2* v1, vec2* v2)
{
	res->x = v1->x + v2->x;
	res->y = v1->y + v2->y;
}

void vec2_sub(vec2* res, vec2* v1, vec2* v2)
{
	res->x = v1->x - v2->x;
	res->y = v1->y - v2->y;
}

void vec2_assign(vec2* res, const vec2* val)
{
	res->x = val->x;
	res->y = val->y;
}

float vec2_len(vec2* val)
{
	return sqrtf((val->x * val->x) + (val->y * val->y));
}

void vec2_norm(vec2* res, vec2* val)
{
	if(!val->x && !val->y)
	{
		vec2_assign(res, val);
		return;
	}

	float l = 1.0f / vec2_len(val);
	vec2 v;
	v.x = val->x * l;
	v.y = val->y * l;

	res->x = v.x;
	res->y = v.y;
}

void vec2_mul(vec2* res, vec2* v1, vec2* v2)
{
	res->x = v1->x * v2->x;
	res->y = v1->y * v2->y;
}

int vec2_equals(vec2* v1, vec2* v2)
{
	if((v1->x < (v2->x + EPSILON) && v1->x >(v2->x - EPSILON)) &&
		(v1->y < (v2->y + EPSILON) && v1->y >(v2->y - EPSILON)))
		return 1;

	return 0;
}

void vec3_fill(vec3* res, float x, float y, float z)
{
	res->x = x;
	res->y = y;
	res->z = z;
}

void vec3_add(vec3* res, const vec3* v1, const vec3* v2)
{
	res->x = v1->x + v2->x;
	res->y = v1->y + v2->y;
	res->z = v1->z + v2->z;
}

void vec3_sub(vec3* res, const vec3* v1, const vec3* v2)
{
	res->x = v1->x - v2->x;
	res->y = v1->y - v2->y;
	res->z = v1->z - v2->z;
}

void vec3_assign(vec3* res, const vec3* val)
{
	res->x = val->x;
	res->y = val->y;
	res->z = val->z;
}

void vec3_cross(vec3* res, const vec3* v1, const vec3* v2)
{
	vec3 v;
	v.x = (v1->y * v2->z) - (v1->z * v2->y);
	v.y = (v1->z * v2->x) - (v1->x * v2->z);
	v.z = (v1->x * v2->y) - (v1->y * v2->x);

	res->x = v.x;
	res->y = v.y;
	res->z = v.z;
}

float vec3_len(vec3* val)
{
	return sqrtf((val->x * val->x) + (val->y * val->y) + (val->z * val->z));
}

float vec3_distance(vec3 p1, vec3 p2)
{
	vec3 v = { 0.f, 0.f, 0.f };
	vec3_sub(&v, &p1, &p2);
	return vec3_len(&v);
}

void vec3_norm(vec3* res, vec3* val)
{
	if(!val->x && !val->y && !val->z)
	{
		vec3_assign(res, val);
		return;
	}

	float l = 1.0f / vec3_len(val);
	vec3 v;
	v.x = val->x * l;
	v.y = val->y * l;
	v.z = val->z * l;

	res->x = v.x;
	res->y = v.y;
	res->z = v.z;
}

void vec3_mul(vec3* res, vec3* v1, vec3* v3)
{
	res->x = v1->x * v3->x;
	res->y = v1->y * v3->y;
	res->z = v1->z * v3->z;
}

void vec3_mul_mat4(vec3* res, vec3* val, mat4* mat)
{
	vec3 v;
	v.x = val->x * mat->mat[0] + val->y * mat->mat[4] + val->z * mat->mat[8] + mat->mat[12];
	v.y = val->x * mat->mat[1] + val->y * mat->mat[5] + val->z * mat->mat[9] + mat->mat[13];
	v.z = val->x * mat->mat[2] + val->y * mat->mat[6] + val->z * mat->mat[10] + mat->mat[14];
	res->x = v.x;
	res->y = v.y;
	res->z = v.z;
}

void vec3_scale(vec3* res, const vec3* val, float s)
{
	res->x = val->x * s;
	res->y = val->y * s;
	res->z = val->z * s;
}

int vec3_equals(vec3* v1, vec3* v2)
{
	if((v1->x < (v2->x + EPSILON) && v1->x >(v2->x - EPSILON)) &&
		(v1->y < (v2->y + EPSILON) && v1->y >(v2->y - EPSILON)) &&
		(v1->z < (v2->z + EPSILON) && v1->z >(v2->z - EPSILON)))
		return 1;

	return 0;
}

void vec3_transform_norm(vec3* res, const vec3* val, const mat4* mat)
{
	/*
	  a = (Vx, Vy, Vz, 0)
	  b = (a×M)T
	  Out = (bx, by, bz)

	  Omits the translation, only scaling + rotating*/
	vec3 v;
	v.x = val->x * mat->mat[0] + val->y * mat->mat[4] + val->z * mat->mat[8];
	v.y = val->x * mat->mat[1] + val->y * mat->mat[5] + val->z * mat->mat[9];
	v.z = val->x * mat->mat[2] + val->y * mat->mat[6] + val->z * mat->mat[10];
	res->x = v.x;
	res->y = v.y;
	res->z = v.z;
}

float vec3_dot(vec3* v1, vec3* v2)
{
	return (v1->x * v2->x +
		v1->y * v2->y +
		v1->z * v2->z);
}

float vec3_angle(vec3* dir1, vec3* dir2)
{
	float dot = vec3_dot(dir1, dir2);
	return TO_DEGREES(acosf(dot));
}


void vec4_fill(vec4* res, float x, float y, float z, float w)
{
	res->x = x;
	res->y = y;
	res->z = z;
	res->w = w;
}

void vec4_fill_vec3(vec4* res, const vec3* v, float w)
{
	res->x = v->x;
	res->y = v->y;
	res->z = v->z;
	res->w = w;
}

void vec4_add(vec4* res, vec4* v1, vec4* v4)
{
	res->x = v1->x + v4->x;
	res->y = v1->y + v4->y;
	res->z = v1->z + v4->z;
	res->w = v1->w + v4->w;
}

void vec4_sub(vec4* res, vec4* v1, vec4* v4)
{
	res->x = v1->x - v4->x;
	res->y = v1->y - v4->y;
	res->z = v1->z - v4->z;
	res->w = v1->w - v4->w;
}

void vec4_assign(vec4* res, const vec4* val)
{
	res->x = val->x;
	res->y = val->y;
	res->z = val->z;
	res->w = val->w;
}

float vec4_len(vec4* val)
{
	return sqrtf((val->x * val->x) +
		(val->y * val->y) +
		(val->z * val->z) +
		(val->w * val->w));
}

void vec4_norm(vec4* res, vec4* val)
{
	if(!val->x && !val->y && !val->z && !val->w)
	{
		vec4_assign(res, val);
		return;
	}

	float l = 1.0f / vec4_len(val);
	vec4 v;
	v.x = val->x * l;
	v.y = val->y * l;
	v.z = val->z * l;
	v.w = val->w * l;

	res->x = v.x;
	res->y = v.y;
	res->z = v.z;
	res->w = v.w;
}

void vec4_mul(vec4* res, vec4* v1, vec4* v4)
{
	res->x = v1->x * v4->x;
	res->y = v1->y * v4->y;
	res->z = v1->z * v4->z;
	res->w = v1->w * v4->w;
}

void vec4_mul_mat4(vec4* res, vec4* val, mat4* mat)
{
	vec4 v;
	v.x = val->x * mat->mat[0] + val->y * mat->mat[4] + val->z * mat->mat[8] + val->w * mat->mat[12];
	v.y = val->x * mat->mat[1] + val->y * mat->mat[5] + val->z * mat->mat[9] + val->w * mat->mat[13];
	v.z = val->x * mat->mat[2] + val->y * mat->mat[6] + val->z * mat->mat[10] + val->w * mat->mat[14];
	v.w = val->x * mat->mat[3] + val->y * mat->mat[7] + val->z * mat->mat[11] + val->w * mat->mat[15];
	res->x = v.x;
	res->y = v.y;
	res->z = v.z;
	res->w = v.w;
}

void vec4_scale(vec4* res, const vec4* val, float s)
{
	res->x = val->x * s;
	res->y = val->y * s;
	res->z = val->z * s;
	res->w = val->w * s;
}

int vec4_equals(vec4* v1, vec4* v2)
{
	if((v1->x < (v2->x + EPSILON) && v1->x >(v2->x - EPSILON)) &&
		(v1->y < (v2->y + EPSILON) && v1->y >(v2->y - EPSILON)) &&
		(v1->z < (v2->z + EPSILON) && v1->z >(v2->z - EPSILON)) &&
		(v1->w < (v2->w + EPSILON) && v1->w >(v2->w - EPSILON)))
		return 1;

	return 0;
}

void vec4_transform_norm(vec4* res, const vec4* val, const mat4* mat)
{
	/*
	  a = (Vx, Vy, Vz, 0)
	  b = (a×M)T
	  Out = (bx, by, bz)

	  Omits the translation, only scaling + rotating*/
	vec4 v;
	v.x = val->x * mat->mat[0] + val->y * mat->mat[4] + val->z * mat->mat[8];
	v.y = val->x * mat->mat[1] + val->y * mat->mat[5] + val->z * mat->mat[9];
	v.z = val->x * mat->mat[2] + val->y * mat->mat[6] + val->z * mat->mat[10];
	res->x = v.x;
	res->y = v.y;
	res->z = v.z;
}

void mat4_identity(mat4* res)
{
	memset(res->mat, 0, sizeof(float) * 16);
	res->mat[0] = res->mat[5] = res->mat[10] = res->mat[15] = 1.0f;
}

void mat4_mul(mat4* res, const mat4* mat1, const mat4* mat2)
{
	float mat[16];
	const float *m1 = mat1->mat, *m2 = mat2->mat;

	mat[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
	mat[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
	mat[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
	mat[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];

	mat[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
	mat[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
	mat[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
	mat[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];

	mat[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
	mat[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
	mat[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
	mat[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];

	mat[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
	mat[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
	mat[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
	mat[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];

	memcpy(res->mat, mat, sizeof(float) * 16);
}

void mat4_translate(mat4* res, float x, float y, float z)
{
	memset(res->mat, 0, sizeof(float) * 16);
	res->mat[0] = 1.0f;
	res->mat[5] = 1.0f;
	res->mat[10] = 1.0f;

	res->mat[12] = x;
	res->mat[13] = y;
	res->mat[14] = z;
	res->mat[15] = 1.0f;
}

void mat4_lookat(mat4* res, const vec3* eye, const vec3* center, const vec3* up_vec)
{
	vec3 f, up, s, u;
	mat4 translate;

	vec3_sub(&f, center, eye);
	vec3_norm(&f, &f);

	vec3_assign(&up, up_vec);
	vec3_norm(&up, &up);

	vec3_cross(&s, &f, &up);
	vec3_norm(&s, &s);

	vec3_cross(&u, &s, &f);
	vec3_norm(&s, &s);

	mat4_identity(res);

	res->mat[0] = s.x;
	res->mat[4] = s.y;
	res->mat[8] = s.z;

	res->mat[1] = u.x;
	res->mat[5] = u.y;
	res->mat[9] = u.z;

	res->mat[2] = -f.x;
	res->mat[6] = -f.y;
	res->mat[10] = -f.z;

	mat4_translate(&translate, -eye->x, -eye->y, -eye->z);
	mat4_mul(res, res, &translate);
}

void mat4_perspective(mat4* res, float fov, float aspect, float nearz, float farz)
{
	float r = TO_RADIANS(fov / 2);
	float deltaz = farz - nearz;
	float s = (float)sin(r);
	float cotangent = 0;

	if(deltaz == 0 || s == 0 || aspect == 0)
		return;

	cotangent = (float)cos(r) / s;
	mat4_identity(res);
	res->mat[0] = cotangent / aspect;
	res->mat[5] = cotangent;
	res->mat[10] = -(farz + nearz) / deltaz;
	res->mat[11] = -1;
	res->mat[14] = -2 * nearz * farz / deltaz;
	res->mat[15] = 0;
}

void mat4_ortho(mat4* res,
	float left, float right,
	float bottom, float top,
	float nearz, float farz)
{
	float tx = -((right + left) / (right - left));
	float ty = -((top + bottom) / (top - bottom));
	float tz = -((farz + nearz) / (farz - nearz));
	mat4_identity(res);
	res->mat[0] = 2 / (right - left);
	res->mat[5] = 2 / (top - bottom);
	res->mat[10] = -2 / (farz - nearz);
	res->mat[12] = tx;
	res->mat[13] = ty;
	res->mat[14] = tz;
}

void mat4_scale(mat4* res, float x, float y, float z)
{
	memset(res->mat, 0, sizeof(float) * 16);
	res->mat[0] = x;
	res->mat[5] = y;
	res->mat[10] = z;
	res->mat[15] = 1.0f;
}

void mat4_from_quat(mat4* res, const quat* q)
{
	float xx = q->x * q->x;
	float xy = q->x * q->y;
	float xz = q->x * q->z;
	float xw = q->x * q->w;

	float yy = q->y * q->y;
	float yz = q->y * q->z;
	float yw = q->y * q->w;

	float zz = q->z * q->z;
	float zw = q->z * q->w;

	res->mat[0] = 1 - 2 * (yy + zz);
	res->mat[1] = 2 * (xy + zw);
	res->mat[2] = 2 * (xz - yw);
	res->mat[3] = 0;

	res->mat[4] = 2 * (xy - zw);
	res->mat[5] = 1 - 2 * (xx + zz);
	res->mat[6] = 2 * (yz + xw);
	res->mat[7] = 0.0;

	res->mat[8] = 2 * (xz + yw);
	res->mat[9] = 2 * (yz - xw);
	res->mat[10] = 1 - 2 * (xx + yy);
	res->mat[11] = 0.0;

	res->mat[12] = 0.0;
	res->mat[13] = 0.0;
	res->mat[14] = 0.0;
	res->mat[15] = 1.0;
}

void mat4_rot_x(mat4* res, const float angle)
{
	float angle_radians = TO_RADIANS(angle);
	res->mat[0] = 1.0f;
	res->mat[1] = 0.0f;
	res->mat[2] = 0.0f;
	res->mat[3] = 0.0f;

	res->mat[4] = 0.0f;
	res->mat[5] = cosf(angle_radians);
	res->mat[6] = sinf(angle_radians);
	res->mat[7] = 0.0f;

	res->mat[8] = 0.0f;
	res->mat[9] = -sinf(angle_radians);
	res->mat[10] = cosf(angle_radians);
	res->mat[11] = 0.0f;

	res->mat[12] = 0.0f;
	res->mat[13] = 0.0f;
	res->mat[14] = 0.0f;
	res->mat[15] = 1.0f;
}

void mat4_rot_y(mat4* res, float angle)
{
	float angle_radians = TO_RADIANS(angle);
	res->mat[0] = cosf(angle_radians);
	res->mat[1] = 0.0f;
	res->mat[2] = -sinf(angle_radians);
	res->mat[3] = 0.0f;

	res->mat[4] = 0.0f;
	res->mat[5] = 1.0f;
	res->mat[6] = 0.0f;
	res->mat[7] = 0.0f;

	res->mat[8] = sinf(angle_radians);
	res->mat[9] = 0.0f;
	res->mat[10] = cosf(angle_radians);
	res->mat[11] = 0.0f;

	res->mat[12] = 0.0f;
	res->mat[13] = 0.0f;
	res->mat[14] = 0.0f;
	res->mat[15] = 1.0f;
}

void mat4_rot_z(mat4* res, float angle)
{
	float angle_radians = TO_RADIANS(angle);
	res->mat[0] = cosf(angle_radians);
	res->mat[1] = sinf(angle_radians);
	res->mat[2] = 0.0f;
	res->mat[3] = 0.0f;

	res->mat[4] = -sinf(angle_radians);
	res->mat[5] = cosf(angle_radians);
	res->mat[6] = 0.0f;
	res->mat[7] = 0.0f;

	res->mat[8] = 0.0f;
	res->mat[9] = 0.0f;
	res->mat[10] = 1.0f;
	res->mat[11] = 0.0f;

	res->mat[12] = 0.0f;
	res->mat[13] = 0.0f;
	res->mat[14] = 0.0f;
	res->mat[15] = 1.0f;
}

void mat4_assign(mat4* res, const mat4* m)
{
	if(res == m)
		return;
	memcpy(res->mat, m->mat, sizeof(float) * 16);
}


void mat4_inverse(mat4* res, mat4* mat)
{
	mat4 tmp;
	float det;
	int i;

	tmp.mat[0] = mat->mat[5] * mat->mat[10] * mat->mat[15] -
		mat->mat[5] * mat->mat[11] * mat->mat[14] -
		mat->mat[9] * mat->mat[6] * mat->mat[15] +
		mat->mat[9] * mat->mat[7] * mat->mat[14] +
		mat->mat[13] * mat->mat[6] * mat->mat[11] -
		mat->mat[13] * mat->mat[7] * mat->mat[10];

	tmp.mat[4] = -mat->mat[4] * mat->mat[10] * mat->mat[15] +
		mat->mat[4] * mat->mat[11] * mat->mat[14] +
		mat->mat[8] * mat->mat[6] * mat->mat[15] -
		mat->mat[8] * mat->mat[7] * mat->mat[14] -
		mat->mat[12] * mat->mat[6] * mat->mat[11] +
		mat->mat[12] * mat->mat[7] * mat->mat[10];

	tmp.mat[8] = mat->mat[4] * mat->mat[9] * mat->mat[15] -
		mat->mat[4] * mat->mat[11] * mat->mat[13] -
		mat->mat[8] * mat->mat[5] * mat->mat[15] +
		mat->mat[8] * mat->mat[7] * mat->mat[13] +
		mat->mat[12] * mat->mat[5] * mat->mat[11] -
		mat->mat[12] * mat->mat[7] * mat->mat[9];

	tmp.mat[12] = -mat->mat[4] * mat->mat[9] * mat->mat[14] +
		mat->mat[4] * mat->mat[10] * mat->mat[13] +
		mat->mat[8] * mat->mat[5] * mat->mat[14] -
		mat->mat[8] * mat->mat[6] * mat->mat[13] -
		mat->mat[12] * mat->mat[5] * mat->mat[10] +
		mat->mat[12] * mat->mat[6] * mat->mat[9];

	tmp.mat[1] = -mat->mat[1] * mat->mat[10] * mat->mat[15] +
		mat->mat[1] * mat->mat[11] * mat->mat[14] +
		mat->mat[9] * mat->mat[2] * mat->mat[15] -
		mat->mat[9] * mat->mat[3] * mat->mat[14] -
		mat->mat[13] * mat->mat[2] * mat->mat[11] +
		mat->mat[13] * mat->mat[3] * mat->mat[10];

	tmp.mat[5] = mat->mat[0] * mat->mat[10] * mat->mat[15] -
		mat->mat[0] * mat->mat[11] * mat->mat[14] -
		mat->mat[8] * mat->mat[2] * mat->mat[15] +
		mat->mat[8] * mat->mat[3] * mat->mat[14] +
		mat->mat[12] * mat->mat[2] * mat->mat[11] -
		mat->mat[12] * mat->mat[3] * mat->mat[10];

	tmp.mat[9] = -mat->mat[0] * mat->mat[9] * mat->mat[15] +
		mat->mat[0] * mat->mat[11] * mat->mat[13] +
		mat->mat[8] * mat->mat[1] * mat->mat[15] -
		mat->mat[8] * mat->mat[3] * mat->mat[13] -
		mat->mat[12] * mat->mat[1] * mat->mat[11] +
		mat->mat[12] * mat->mat[3] * mat->mat[9];

	tmp.mat[13] = mat->mat[0] * mat->mat[9] * mat->mat[14] -
		mat->mat[0] * mat->mat[10] * mat->mat[13] -
		mat->mat[8] * mat->mat[1] * mat->mat[14] +
		mat->mat[8] * mat->mat[2] * mat->mat[13] +
		mat->mat[12] * mat->mat[1] * mat->mat[10] -
		mat->mat[12] * mat->mat[2] * mat->mat[9];

	tmp.mat[2] = mat->mat[1] * mat->mat[6] * mat->mat[15] -
		mat->mat[1] * mat->mat[7] * mat->mat[14] -
		mat->mat[5] * mat->mat[2] * mat->mat[15] +
		mat->mat[5] * mat->mat[3] * mat->mat[14] +
		mat->mat[13] * mat->mat[2] * mat->mat[7] -
		mat->mat[13] * mat->mat[3] * mat->mat[6];

	tmp.mat[6] = -mat->mat[0] * mat->mat[6] * mat->mat[15] +
		mat->mat[0] * mat->mat[7] * mat->mat[14] +
		mat->mat[4] * mat->mat[2] * mat->mat[15] -
		mat->mat[4] * mat->mat[3] * mat->mat[14] -
		mat->mat[12] * mat->mat[2] * mat->mat[7] +
		mat->mat[12] * mat->mat[3] * mat->mat[6];

	tmp.mat[10] = mat->mat[0] * mat->mat[5] * mat->mat[15] -
		mat->mat[0] * mat->mat[7] * mat->mat[13] -
		mat->mat[4] * mat->mat[1] * mat->mat[15] +
		mat->mat[4] * mat->mat[3] * mat->mat[13] +
		mat->mat[12] * mat->mat[1] * mat->mat[7] -
		mat->mat[12] * mat->mat[3] * mat->mat[5];

	tmp.mat[14] = -mat->mat[0] * mat->mat[5] * mat->mat[14] +
		mat->mat[0] * mat->mat[6] * mat->mat[13] +
		mat->mat[4] * mat->mat[1] * mat->mat[14] -
		mat->mat[4] * mat->mat[2] * mat->mat[13] -
		mat->mat[12] * mat->mat[1] * mat->mat[6] +
		mat->mat[12] * mat->mat[2] * mat->mat[5];

	tmp.mat[3] = -mat->mat[1] * mat->mat[6] * mat->mat[11] +
		mat->mat[1] * mat->mat[7] * mat->mat[10] +
		mat->mat[5] * mat->mat[2] * mat->mat[11] -
		mat->mat[5] * mat->mat[3] * mat->mat[10] -
		mat->mat[9] * mat->mat[2] * mat->mat[7] +
		mat->mat[9] * mat->mat[3] * mat->mat[6];

	tmp.mat[7] = mat->mat[0] * mat->mat[6] * mat->mat[11] -
		mat->mat[0] * mat->mat[7] * mat->mat[10] -
		mat->mat[4] * mat->mat[2] * mat->mat[11] +
		mat->mat[4] * mat->mat[3] * mat->mat[10] +
		mat->mat[8] * mat->mat[2] * mat->mat[7] -
		mat->mat[8] * mat->mat[3] * mat->mat[6];

	tmp.mat[11] = -mat->mat[0] * mat->mat[5] * mat->mat[11] +
		mat->mat[0] * mat->mat[7] * mat->mat[9] +
		mat->mat[4] * mat->mat[1] * mat->mat[11] -
		mat->mat[4] * mat->mat[3] * mat->mat[9] -
		mat->mat[8] * mat->mat[1] * mat->mat[7] +
		mat->mat[8] * mat->mat[3] * mat->mat[5];

	tmp.mat[15] = mat->mat[0] * mat->mat[5] * mat->mat[10] -
		mat->mat[0] * mat->mat[6] * mat->mat[9] -
		mat->mat[4] * mat->mat[1] * mat->mat[10] +
		mat->mat[4] * mat->mat[2] * mat->mat[9] +
		mat->mat[8] * mat->mat[1] * mat->mat[6] -
		mat->mat[8] * mat->mat[2] * mat->mat[5];

	det = mat->mat[0] * tmp.mat[0] + mat->mat[1] * tmp.mat[4] + mat->mat[2] * tmp.mat[8] + mat->mat[3] * tmp.mat[12];

	if(det == 0) {
		return;
	}

	det = 1.f / det;

	for(i = 0; i < 16; i++) {
		res->mat[i] = tmp.mat[i] * det;
	}
}




void quat_fill(quat* res, float x, float y, float z, float w)
{
	res->x = x;
	res->y = y;
	res->z = z;
	res->w = w;
}

void quat_identity(quat* res)
{
	res->x = 0.0;
	res->y = 0.0;
	res->z = 0.0;
	res->w = 1.0;
}

void quat_mul_vec3(vec3* res, const quat* q, const vec3* v)
{
	vec3 uv, uuv, qvec;
	qvec.x = q->x;
	qvec.y = q->y;
	qvec.z = q->z;

	vec3_cross(&uv, &qvec, v);
	vec3_cross(&uuv, &qvec, &uv);

	vec3_scale(&uv, &uv, (2.0f * q->w));
	vec3_scale(&uuv, &uuv, 2.0f);

	vec3_add(res, v, &uv);
	vec3_add(res, res, &uuv);
}

void quat_assign(quat* res, const quat* val)
{
	res->x = val->x;
	res->y = val->y;
	res->z = val->z;
	res->w = val->w;
}

void quat_mul(quat* res, const quat* q1, const quat* q2)
{
	quat tmp1, tmp2;
	quat_assign(&tmp1, q1);
	quat_assign(&tmp2, q2);

	res->x = tmp1.w * tmp2.x + tmp1.x * tmp2.w + tmp1.y * tmp2.z - tmp1.z * tmp2.y;
	res->y = tmp1.w * tmp2.y + tmp1.y * tmp2.w + tmp1.z * tmp2.x - tmp1.x * tmp2.z;
	res->z = tmp1.w * tmp2.z + tmp1.z * tmp2.w + tmp1.x * tmp2.y - tmp1.y * tmp2.x;
	res->w = tmp1.w * tmp2.w - tmp1.x * tmp2.x - tmp1.y * tmp2.y - tmp1.z * tmp2.z;
}

float quat_len_sq(const quat* q)
{
	return (q->x * q->x) + (q->y * q->y) + (q->z * q->z) + (q->w * q->w);
}

float quat_len(const quat* q)
{
	return (float)sqrt(quat_len_sq(q));
}

void quat_norm(quat* res, const quat* val)
{
	float length = quat_len(val);
	if(fabs(length) < EPSILON)
	{
		quat_fill(res, 0.0f, 0.0f, 0.0f, 0.0f);
		return;
	}

	quat_fill(res, res->x / length, res->y / length, res->z / length, res->w / length);
}

void quat_axis_angle(quat* res, const vec3* v, float angle)
{
	float half_angle = TO_RADIANS(angle) * 0.5f;
	float scale = sinf(half_angle);

	res->x = v->x * scale;
	res->y = v->y * scale;
	res->z = v->z * scale;
	res->w = cosf(half_angle);
	quat_norm(res, res);
}

void quat_get_forward_rh(vec3* res, const quat* q)
{
	quat_mul_vec3(res, q, &UNIT_Z_NEG);
}

void quat_get_forward_lh(vec3* res, const quat* q)
{
	quat_mul_vec3(res, q, &UNIT_Z);
}

void quat_get_up(vec3* res, const quat* q)
{
	quat_mul_vec3(res, q, &UNIT_Y);
}

void quat_get_right(vec3* res, const quat* q)
{
	quat_mul_vec3(res, q, &UNIT_X);
}

float quat_get_pitch(const quat* q)
{
	float check = 2.0f * (-q->y * q->z + q->w * q->x);
	if(check < -0.995f)
		return -90.f;
	else if(check > 0.995)
		return 90.f;
	else
		return TO_DEGREES(asinf(check));
}

float quat_get_yaw(const quat* q)
{
	float check = 2.0f * (-q->y * q->z + q->w * q->x);
	if(check > 0.995f || check < -0.995f)
		return 0.f;
	else
		return TO_DEGREES(atan2f(2.0f * (q->x * q->z + q->w * q->y), 1.0f - 2.0f * (q->x * q->x + q->y * q->y)));
}

float quat_get_roll(const quat* q)
{
	float check = 2.0f * (-q->y * q->z + q->w * q->x);
	if(check < -0.955f)
		return TO_DEGREES(-atan2f(2.0f * (q->x * q->z - q->w * q->y), 1.0f - 2.0f * (q->y * q->y + q->z * q->z)));
	else if(check > 0.995f)
		return TO_DEGREES(atan2f(2.0f * (q->x * q->z - q->w * q->y), 1.0f - 2.0f * (q->y * q->y + q->z * q->z)));
	else
		return TO_DEGREES(atan2f(2.0f * (q->x * q->y + q->w * q->z), 1.0f - 2.0f * (q->x * q->x + q->z * q->z)));
}

void quat_mul_mat4(quat* res, quat* val, mat4* mat)
{
	vec4 v;
	vec4_fill(&v, val->x, val->y, val->z, val->w);
	vec4_mul_mat4(&v, &v, mat);
	res->x = v.x;
	res->y = v.y;
	res->z = v.z;
	res->w = v.w;
}

void plane_init(Plane* plane, vec3* normal, vec3* point)
{
	vec3_assign(&plane->normal, normal);
	float dot = vec3_dot(&plane->normal, point);
	plane->constant = -dot;
}
