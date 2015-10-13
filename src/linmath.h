#ifndef LINMATH_H
#define LINMATH_H

#include "../include/kazmath/kazmath/kazmath.h"
#include "../include/kazmath/kazmath/vec4.h"
#include <math.h>

/* Just for convenience & consistency */
typedef kmVec2        vec2;
typedef kmVec3        vec3;
typedef struct kmVec4 vec4;
typedef kmMat3        mat3;
typedef kmMat4        mat4;
typedef kmQuaternion  quat;

/* vec2 */
#define vec2_fill           kmVec2Fill
#define vec2_add            kmVec2Add
#define vec2_assign         kmVec2Assign
#define vec2_norm           kmVec2Normalize
#define vec2_mul            kmVec2Mul

/* vec3 */
#define vec3_fill           kmVec3Fill
#define vec3_add            kmVec3Add
#define vec3_assign         kmVec3Assign
#define vec3_norm           kmVec3Normalize
#define vec3_mul            kmVec3Mul
#define vec3_mul_mat4       kmVec3MultiplyMat4
#define vec3_mul_mat3       kmVec3MultiplyMat3
#define vec3_scale          kmVec3Scale
#define vec3_equals         kmVec3Equals
#define vec3_transform      kmVec3Transform
#define vec3_transform_norm kmVec3TransformNormal
#define vec3_len            kmVec3Length

/* vec4 */
#define vec4_fill           kmVec4Fill
#define vec4_add            kmVec4Add
#define vec4_assign         kmVec4Assign
#define vec4_norm           kmVec4Normalize
#define vec4_mul            kmVec4Mul
#define vec4_mul_mat4       kmVec4MultiplyMat4
#define vec4_scale          kmVec4Scale
#define vec4_equals         kmVec4Equals
#define vec4_transform      kmVec4Transform
#define vec4_transform_norm kmVec4TransformNormal
#define vec4_len            kmVec4Length

/* mat4 */
#define mat4_identity    kmMat4Identity
#define mat4_mul         kmMat4Multiply
#define mat4_lookat      kmMat4LookAt
#define mat4_perspective kmMat4PerspectiveProjection
#define mat4_ortho       kmMat4OrthographicProjection
#define mat4_scale       kmMat4Scaling
#define mat4_translate   kmMat4Translation
#define mat4_from_quat   kmMat4RotationQuaternion
#define mat4_rot_x       kmMat4RotationX
#define mat4_rot_y       kmMat4RotationY
#define mat4_rot_z       kmMat4RotationZ
#define mat4_assign      kmMat4Assign
#define mat4_assign_mat3 kmMat4AssignMat3

/* quat */
#define quat_identity       kmQuaternionIdentity
#define quat_mul_vec3       kmQuaternionMultiplyVec3
#define quat_mul            kmQuaternionMultiply
#define quat_axis_angle     kmQuaternionRotationAxisAngle
#define quat_get_forward_rh kmQuaternionGetForwardVec3RH
#define quat_get_forward_lh kmQuaternionGetForwardVec3LH
#define quat_get_up         kmQuaternionGetUpVec3
#define quat_get_right      kmQuaternionGetRightVec3

/* Conversions */
#define M_PI		        3.14159265358979323846 
#define TO_RADIANS(degrees) ((degrees * M_PI) / 180.0)
#define TO_DEGREES(radians) ((radians * 180.0) / M_PI)

#endif
