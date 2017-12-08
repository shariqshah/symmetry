#ifndef PHYSICS_H
#define PHYSICS_H

#include "../common/common.h"

void physics_init(void);
void physics_cleanup(void);
void physics_gravity_set(float x, float y, float z);
void physics_gravity_get(float* x, float* y, float* z);
void physics_step(float delta_time);

void physics_body_position_get(Rigidbody body, float* x, float* y, float* z);
void physics_body_position_set(Rigidbody body, float x, float y, float z);

void physics_body_rotation_get(Rigidbody body, float* x, float* y, float* z, float* w);
void physics_body_rotation_set(Rigidbody body, float x, float y, float z, float w);

Collision_Shape physics_body_cs_get(Rigidbody body);
void            physics_body_cs_set(Rigidbody body, Collision_Shape shape);

void            physics_cs_remove(Collision_Shape shape);

Collision_Shape physics_cs_plane_create(float a, float b, float c, float d);
void            physics_cs_plane_params_set(Collision_Shape shape, float a, float b, float c, float d);
void            physics_cs_plane_params_get(Collision_Shape shape, float* a, float* b, float* c, float* d);

Collision_Shape physics_cs_box_create(float x, float y, float z);
void            physics_cs_box_params_get(Collision_Shape shape, float* x, float* y, float* z);
void            physics_cs_box_params_set(Collision_Shape shape, float x, float y, float z);

Collision_Shape physics_cs_sphere_create(float radius);
float           physics_cs_sphere_radius_get(Collision_Shape shape);
void            physics_cs_sphere_radius_set(Collision_Shape shape, float radius);

Collision_Shape physics_cs_capsule_create(float radius, float height);
void            physics_cs_capsule_params_get(Collision_Shape shape, float* radius, float* height);
void            physics_cs_capsule_params_set(Collision_Shape shape, float radius, float height);

Rigidbody physics_body_box_create(float x, float y, float z);
Rigidbody physics_body_sphere_create(float radius);
Rigidbody physics_body_capsule_create(float radius, float height);

void physics_body_remove(Rigidbody body);

void physics_body_set_moved_callback(RigidbodyMoveCB callback);
void physics_body_set_collision_callback(RigidbodyColCB callback);

float physics_body_mass_get(Rigidbody body);
void  physics_body_mass_set(Rigidbody body, float mass);

void  physics_body_data_set(Rigidbody body, void* data);
void* physics_body_data_get(Rigidbody body);

void physics_body_force_add(Rigidbody body, float fx, float fy, float fz);

void physics_body_kinematic_set(Rigidbody body);

#endif