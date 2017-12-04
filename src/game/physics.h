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

Rigidbody physics_plane_create(float a, float b, float c, float d);
Rigidbody physics_box_create(float length, float width, float height);

void physics_body_set_moved_callback(RigidbodyMoveCB callback);
void physics_body_set_collision_callback(RigidbodyColCB callback);

float physics_body_mass_get(Rigidbody body);
void  physics_body_mass_set(Rigidbody body, float mass);

void  physics_body_data_set(Rigidbody body, void* data);
void* physics_body_data_get(Rigidbody body);

void physics_body_kinematic_set(Rigidbody body);

#endif