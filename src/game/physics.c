#include "physics.h"
#include "../common/log.h"

#include <ode/ode.h>
#include <assert.h>

static struct 
{
	dWorldID world;
	dSpaceID space;
	dJointGroupID contact_group;
	double step_size;
}
Physics;

static void physics_near_callback(void* data, dGeomID body1, dGeomID body2);

void physics_init(void)
{
	if(dInitODE2(0) == 0)
	{
		log_error("physics:init", "Failed to initialize ode");
		return;
	}

	dAllocateODEDataForThread(dAllocateMaskAll);
	Physics.world = dWorldCreate();
	if(!Physics.world)
	{
		log_error("physics:init", "Physics world created!");
	}
	else
	{
		log_message("Physics world created");
		Physics.space = dHashSpaceCreate(0);
		Physics.contact_group = dJointGroupCreate(0);
		Physics.step_size = 0.16;
	}
}

void physics_cleanup(void)
{
	dJointGroupDestroy(Physics.contact_group);
	dSpaceDestroy(Physics.space);
	if(Physics.world)
	{
		dWorldDestroy(Physics.world);
	}
	dCleanupODEAllDataForThread();
	dCloseODE();
}

void physics_step(float delta_time)
{
	int steps = (int)ceilf(delta_time / Physics.step_size);
	for(int i = 0; i < steps; i++)
	{
		dSpaceCollide(Physics.space, NULL, physics_near_callback);
		dWorldQuickStep(Physics.world, Physics.step_size);
		dJointGroupEmpty(Physics.contact_group);
	}
}

void physics_gravity_set(float x, float y, float z)
{
	dWorldSetGravity(Physics.world, x, y, z);
}

void physics_gravity_get(float * x, float * y, float * z)
{
	assert(x && y && z);
	dVector3 gravity;
	dWorldGetGravity(Physics.world, gravity);
	*x = gravity[0];
	*y = gravity[1];
	*z = gravity[2];
}

void physics_body_position_get(Rigidbody body, float * x, float * y, float * z)
{
	assert(x && y && z);
	const dReal* position = dBodyGetPosition(body);
	*x = position[0];
	*y = position[1];
	*z = position[2];
}

void physics_body_position_set(Rigidbody body, float x, float y, float z)
{
	dBodySetPosition(body, x, y, z);
}

void physics_body_rotation_get(Rigidbody body, float * x, float * y, float * z, float * w)
{
	assert(x && y && z && w);
	const dReal* rotation = dBodyGetQuaternion(body);
	*x = rotation[1];
	*y = rotation[2];
	*z = rotation[3];
	*w = rotation[0];
}

void physics_body_rotation_set(Rigidbody body, float x, float y, float z, float w)
{
	dReal rotation[4] = { 0 };
	rotation[1] = x;
	rotation[2] = y;
	rotation[3] = z;
	rotation[0] = w;
	dBodySetQuaternion(body, &rotation[0]);
}


void physics_near_callback(void* data, dGeomID o1, dGeomID o2)
{
	assert(o1);
	assert(o2);

	if(dGeomIsSpace(o1) || dGeomIsSpace(o2))
	{
		fprintf(stderr, "testing space %p %p\n", (void*)o1, (void*)o2);
		// colliding a space with something
		dSpaceCollide2(o1, o2, data, &physics_near_callback);
		// Note we do not want to test intersections within a space,
		// only between spaces.
		return;
	}

	//  fprintf(stderr,"testing geoms %p %p\n", o1, o2);

	const int N = 32;
	dContact contact[32];
	int n = dCollide(o1, o2, N, &(contact[0].geom), sizeof(dContact));
	if(n > 0)
	{
		for(int i = 0; i<n; i++)
		{
			// Paranoia  <-- not working for some people, temporarily removed for 0.6
			//dIASSERT(dVALIDVEC3(contact[i].geom.pos));
			//dIASSERT(dVALIDVEC3(contact[i].geom.normal));
			//dIASSERT(!dIsNan(contact[i].geom.depth));
			contact[i].surface.slip1 = 0.7;
			contact[i].surface.slip2 = 0.7;
			contact[i].surface.mode = dContactSoftERP | dContactSoftCFM | dContactApprox1 | dContactSlip1 | dContactSlip2;
			contact[i].surface.mu = 50.0; // was: dInfinity
			contact[i].surface.soft_erp = 0.96;
			contact[i].surface.soft_cfm = 0.04;
			dJointID c = dJointCreateContact(Physics.world, Physics.contact_group, &contact[i]);
			dJointAttach(c,
				dGeomGetBody(contact[i].geom.g1),
				dGeomGetBody(contact[i].geom.g2));
		}
	}
}


Rigidbody physics_plane_create(float a, float b, float c, float d)
{
	dGeomID plane = dCreatePlane(Physics.space, a, b, c, d);
	/*dBodyID body = dBodyCreate(Physics.world);
	dGeomSetBody(plane, body);*/
	return plane;
}

Rigidbody physics_box_create(float length, float width, float height)
{
	dGeomID box = dCreateBox(Physics.space, length, height, width);
	dBodyID body = dBodyCreate(Physics.world);
	dGeomSetBody(box, body);
	return body;
}

void physics_body_set_moved_callback(Rigidbody body, RigidbodyMoveCB callback)
{
	dBodySetMovedCallback(body, callback);
}

float physics_body_mass_get(Rigidbody body)
{
	return 0.0f;
}

void physics_body_mass_set(Rigidbody body, float mass)
{
	
}

void physics_body_kinematic_set(Rigidbody body)
{
	dBodySetKinematic(body);
}