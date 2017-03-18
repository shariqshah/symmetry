#ifndef LINMATH_H
#define LINMATH_H


/*
Most code here is from Kazmath library(https://github.com/Kazade/kazmath) and
Linmath tutorials at http://www.euclideanspace.com/ with some minor changes and additions
of my own to make the code consistent. I only copied the necessary parts in the hope that
i'll make my own additions like SIMD etc later on.
 */

/* conversions */
#define EPSILON             0.0001
#define M_PI		        3.14159265358979323846 
#define TO_RADIANS(degrees) ((degrees * M_PI) / 180.0)
#define TO_DEGREES(radians) ((radians * 180.0) / M_PI)

typedef struct vec2_t
{
	float x;
	float y;
} vec2;

typedef struct vec3_t
{
	float x;
	float y;
	float z;
} vec3;

typedef struct vec4_t
{
	float x;
	float y;
	float z;
	float w;
} vec4;

typedef struct mat4_t
{
	float mat[16];
} mat4;

typedef struct quat_t
{
	float x;
	float y;
	float z;
	float w;
} quat;

/* constants */
extern const vec3 UNIT_X;
extern const vec3 UNIT_Y;
extern const vec3 UNIT_Z;

extern const vec3 UNIT_X_NEG;
extern const vec3 UNIT_Y_NEG;
extern const vec3 UNIT_Z_NEG;

/* vec2 */
void  vec2_fill(vec2* res, float x, float y);
void  vec2_add(vec2* res, vec2* v1, vec2* v2);
void  vec2_sub(vec2* res, vec2* v1, vec2* v2);
void  vec2_assign(vec2* res, vec2* val);
void  vec2_mul(vec2* res, vec2* v1, vec2* v2);
float vec2_len(vec2* val);void vec2_norm(vec2* res, vec2* val);

/* vec3 */
void  vec3_fill(vec3* res, float x, float y, float z);
void  vec3_add(vec3* res, const vec3* v1, const vec3* v3);
void  vec3_sub(vec3* res, const vec3* v1, const vec3* v3);
void  vec3_assign(vec3* res, const vec3* val);
void  vec3_cross(vec3* res, const vec3* v1, const vec3* v2);
void  vec3_norm(vec3* res, vec3* val);
void  vec3_mul(vec3* res, vec3* v1, vec3* v3);
void  vec3_mul_mat4(vec3* res, vec3* val, mat4* mat);
void  vec3_scale(vec3* res, const vec3* val, float s);
void  vec3_transform_norm(vec3* res, const vec3* val, const mat4* mat);
int   vec3_equals(vec3* v1, vec3* v2);
float vec3_len(vec3* val);
float vec3_dot(vec3* v1, vec3* v2);

/* vec4 */
int vec4_equals(vec4* v1, vec4* v2);
void vec4_fill(vec4* res, float x, float y, float z, float w);
void vec4_fill_vec3(vec4* res, const vec3* v, float w);
void vec4_transform_norm(vec4* res, const vec4* val, const mat4* mat);
void vec4_scale(vec4* res, const vec4* val, float s);
void vec4_mul_mat4(vec4* res, vec4* val, mat4* mat);
void vec4_mul(vec4* res, vec4* v1, vec4* v4);
void vec4_norm(vec4* res, vec4* val);
float vec4_len(vec4* val);
void vec4_assign(vec4* res, vec4* val);
void vec4_sub(vec4* res, vec4* v1, vec4* v4);
void vec4_add(vec4* res, vec4* v1, vec4* v4);

/* mat4 */
void mat4_assign(mat4* res, const mat4* m);
void mat4_rot_z(mat4* res, float angle);
void mat4_rot_y(mat4* res, float angle);
void mat4_rot_x(mat4* res, const float angle);
void mat4_from_quat(mat4* res, const quat* q);
void mat4_scale(mat4* res, float x, float y, float z);
void mat4_ortho(mat4* res,
				float left,   float right,
				float bottom, float top,
				float nearz,  float farz);
void mat4_perspective(mat4* res, float fov, float aspect, float nearz, float farz);
void mat4_lookat(mat4* res, const vec3* eye, const vec3* center, const vec3* up_vec);
void mat4_translate(mat4* res, float x, float y, float z);
void mat4_mul(mat4* res, const mat4* mat1, const mat4* mat2);
void mat4_identity(mat4* res);
void mat4_inverse(mat4* res, mat4* mat);

/* quat */
float quat_get_roll(const quat* q);
float quat_get_yaw(const quat* q);
float quat_get_pitch(const quat* q);
void  quat_get_right(vec3* res, const quat* q);
void  quat_get_up(vec3* res, const quat* q);
void  quat_get_forward_lh(vec3* res, const quat* q);
void  quat_get_forward_rh(vec3* res, const quat* q);
void  quat_axis_angle(quat* res, const vec3* v, float angle);
void  quat_norm(quat* res, const quat* val);
float quat_len(const quat* q);
float quat_len_sq(const quat* q);
void  quat_mul(quat* res, const quat* q1, const quat* q2);
void  quat_mul_vec3(vec3* res, const quat* q, const vec3* v);
void  quat_assign(quat* res, const quat* val);
void  quat_identity(quat* res);
void  quat_fill(quat* res, float x, float y, float z, float w);

#endif
