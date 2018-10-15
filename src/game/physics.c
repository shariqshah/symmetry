#include "physics.h"
#include "../common/log.h"

#include <ode/ode.h>
#include <assert.h>

static struct 
{
    dWorldID        world;
    dSpaceID        space;
    dJointGroupID   contact_group;
    double          step_size;
    RigidbodyColCB  on_collision;
    RigidbodyMoveCB on_move;
}
    Physics;

static void physics_near_callback(void* data, dGeomID body1, dGeomID body2);
static void physics_cs_ray_hit_callback(void* data, dGeomID geom1, dGeomID geom2);

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
	Physics.step_size = 0.016;
	dWorldSetCFM(Physics.world,1e-5);
	dWorldSetAutoDisableFlag(Physics.world, 1);
	dWorldSetLinearDamping(Physics.world, 0.00001);
	dWorldSetAngularDamping(Physics.world, 0.005);
	dWorldSetMaxAngularSpeed(Physics.world, 200);
	dWorldSetContactMaxCorrectingVel(Physics.world,0.1);
	dWorldSetContactSurfaceLayer(Physics.world,0.001);
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
    int steps = (int)ceilf(delta_time / (float)Physics.step_size);
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
    *x = (float)gravity[0];
    *y = (float)gravity[1];
    *z = (float)gravity[2];
}

void physics_body_position_get(Rigidbody body, float * x, float * y, float * z)
{
    assert(x && y && z);
    const dReal* position = dBodyGetPosition(body);
    *x = (float)position[0];
    *y = (float)position[1];
    *z = (float)position[2];
}

void physics_body_position_set(Rigidbody body, float x, float y, float z)
{
    dBodySetPosition(body, x, y, z);
    dBodyEnable(body);
}

void physics_body_rotation_get(Rigidbody body, float * x, float * y, float * z, float * w)
{
    assert(x && y && z && w);
    const dReal* rotation = dBodyGetQuaternion(body);
    *x = (float)rotation[1];
    *y = (float)rotation[2];
    *z = (float)rotation[3];
    *w = (float)rotation[0];
}

void physics_body_rotation_set(Rigidbody body, float x, float y, float z, float w)
{
    dReal rotation[4] = { 0.f };
    rotation[1] = x;
    rotation[2] = y;
    rotation[3] = z;
    rotation[0] = w;
    dBodySetQuaternion(body, &rotation[0]);
    dBodyEnable(body);
}


void physics_near_callback(void* data, dGeomID o1, dGeomID o2)
{
    assert(o1);
    assert(o2);

    //if(dGeomIsSpace(o1) || dGeomIsSpace(o2))
    //{
    //	fprintf(stderr, "testing space %p %p\n", (void*)o1, (void*)o2);
    //	// colliding a space with something
    //	dSpaceCollide2(o1, o2, data, &physics_near_callback);
    //	// Note we do not want to test intersections within a space,
    //	// only between spaces.
    //	return;
    //}

    //  fprintf(stderr,"testing geoms %p %p\n", o1, o2);

    // exit without doing anything if the two bodies are connected by a joint
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    if (b1 && b2 && dAreConnectedExcluding(b1,b2,dJointTypeContact))
	return;

    if(o1 == o2)
	log_message("same body!");


#define MAX_CONTACTS 8
    dContact contact[MAX_CONTACTS];
    for (int i = 0; i < MAX_CONTACTS; i++)
    {
	//contact[i].surface.mode = dContactBounce | dContactSoftCFM;
	contact[i].surface.mode = dContactBounce | dContactSoftCFM;
	contact[i].surface.mu = dInfinity;
	contact[i].surface.mu2 = 0;
	contact[i].surface.bounce = 0.1;
	contact[i].surface.bounce_vel = 0.1;
	contact[i].surface.soft_cfm = 0.01;
    }

    int n = dCollide(o1, o2, MAX_CONTACTS, &(contact[0].geom), sizeof(dContact));
    if(n > 0)
    {
	Physics.on_collision(b1, b2);
	for(int i = 0; i < n; i++)
	{	
	    //contact[i].surface.slip1 = 0.7;
	    //contact[i].surface.slip2 = 0.7;
	    //contact[i].surface.mode = dContactSoftERP | dContactSoftCFM | dContactApprox1 | dContactSlip1 | dContactSlip2;
	    //contact[i].surface.mu = 50.0; // was: dInfinity
	    //contact[i].surface.soft_erp = 0.96;
	    //contact[i].surface.soft_cfm = 0.04;

	    dJointID c = dJointCreateContact(Physics.world, Physics.contact_group, &contact[i]);
	    dJointAttach(c, b1, b2);
	    /*dGeomGetBody(contact[i].geom.g1),
	      dGeomGetBody(contact[i].geom.g2));*/
	}
    }
}

Collision_Shape physics_body_cs_get(Rigidbody body)
{
    return dBodyGetFirstGeom(body);
}

void physics_body_cs_set(Rigidbody body, Collision_Shape shape)
{
    dGeomSetBody(shape, body);
}

Collision_Shape physics_cs_plane_create(float a, float b, float c, float d)
{
    dGeomID plane = dCreatePlane(Physics.space, a, b, c, d);
    return plane;
}

void physics_cs_plane_params_set(Collision_Shape shape, float a, float b, float c, float d)
{
    dGeomPlaneSetParams(shape, a, b, c, d);
}

void physics_cs_plane_params_get(Collision_Shape shape, float* a, float* b, float* c, float* d)
{
    assert(a && b && c && d);
    *a = *b = *c = *d = 0.f;
    float result[4] = { 0.f };
    dGeomPlaneGetParams(shape, result);
    *a = result[0];
    *b = result[1];
    *c = result[2];
    *d = result[3];
}

Collision_Shape physics_cs_box_create(float x, float y, float z)
{
    dGeomID box = dCreateBox(Physics.space, x, y, z);
    if(!box)
    {
	log_error("physics:cs_box_create", "Failed to create box collision shape");
    }
    return box;

}

int physics_cs_type_get(Collision_Shape shape)
{
    int geom_class = dGeomGetClass(shape);
    switch(geom_class)
    {
    case dBoxClass:      return CST_BOX;
    case dSphereClass:   return CST_SPHERE;
    case dCylinderClass: return CST_CYLINDER; 
    case dTriMeshClass:  return CST_TRIMESH;
    case dPlaneClass:    return CST_PLANE;
    default: return CST_UNKNOWN;
    }
}

void physics_cs_box_params_get(Collision_Shape shape, float* x, float* y, float* z)
{
    assert(x && y && z);
    *x = *y = *z = 0.f;
    dReal lengths[3] = { 0 };
    dGeomBoxGetLengths(shape, &lengths[0]);
    *x = lengths[0];
    *y = lengths[1];
    *z = lengths[2];
}

void physics_cs_box_params_set(Collision_Shape shape, float x, float y, float z)
{
    dGeomBoxSetLengths(shape, x, y, z);
}

Collision_Shape physics_cs_sphere_create(float radius)
{
    dGeomID sphere = dCreateSphere(Physics.space, radius);
    if(!sphere)
    {
	log_error("physics:cs_sphere_create", "Failed to create sphere collision shape");
    }
    return sphere;
}

float physics_cs_sphere_radius_get(Collision_Shape shape)
{
    return dGeomSphereGetRadius(shape);
}

void physics_cs_sphere_radius_set(Collision_Shape shape, float radius)
{
    dGeomSphereSetRadius(shape, radius);
}

Collision_Shape physics_cs_capsule_create(float radius, float length)
{
    dGeomID capsule = dCreateCapsule(Physics.space, radius, length);
    if(!capsule)
    {
	log_error("physics:cs_capsule_create", "Failed to create capsule collision shape");
    }
    return capsule;
}

void physics_cs_capsule_params_get(Collision_Shape shape, float* radius, float* length)
{
    assert(radius && length);
    *radius = *length = 0.f;
    dGeomCapsuleGetParams(shape, radius, length);
}

void physics_cs_capsule_params_set(Collision_Shape shape, float radius, float length)
{
    dGeomCapsuleSetParams(shape, radius, length);
}

void physics_box_params_get(Rigidbody body, float* x, float* y, float* z)
{
    assert(x && y && z);
    *x = *y = *z = 0.f;
    dGeomID box = dBodyGetFirstGeom(body);
    if(box == 0)
    {
	log_error("physics:box_get_params", "Body has invalid geometry");
	return;
    }
    float lengths[3] = { 0.f };
    dGeomBoxGetLengths(box, lengths);
    *x = lengths[0];
    *y = lengths[1];
    *z = lengths[2];
}

void physics_box_params_set(Rigidbody body, float x, float y, float z)
{
    assert(x && y && z);
    dGeomID box = dBodyGetFirstGeom(body);
    if(box == 0)
    {
	log_error("physics:box_get_params", "Body has invalid geometry");
	return;
    }
    dGeomBoxSetLengths(box, x, y, z);
}

Rigidbody physics_body_box_create(float x, float y, float z)
{
    Rigidbody body = NULL;
    Collision_Shape box = physics_cs_box_create(x, y, z);
    if(box)
    {
	body = dBodyCreate(Physics.world);
	physics_body_cs_set(body, box);
	dBodySetMovedCallback(body, Physics.on_move);
    }

    return body;
}

Rigidbody physics_body_sphere_create(float radius)
{
    Rigidbody body = NULL;
    Collision_Shape sphere = physics_cs_sphere_create(radius);
    if(sphere)
    {
	body = dBodyCreate(Physics.world);
	physics_body_cs_set(body, sphere);
	dBodySetMovedCallback(body, Physics.on_move);
    }
	
    return body;
}

Rigidbody physics_body_capsule_create(float radius, float length)
{
    Rigidbody body = NULL;
    Collision_Shape capsule = physics_cs_capsule_create(radius, length);
    if(capsule)
    {
	body = dBodyCreate(Physics.world);
	physics_body_cs_set(body, capsule);
	dBodySetMovedCallback(body, Physics.on_move);
    }

    return body;
}

void physics_body_force_add(Rigidbody body, float fx, float fy, float fz)
{
    dBodyAddForce(body, fx, fy, fz);
}

void physics_body_set_moved_callback(RigidbodyMoveCB callback)
{
    Physics.on_move = callback;
}

void physics_body_set_collision_callback(RigidbodyColCB callback)
{
    Physics.on_collision = callback;
}

float physics_body_mass_get(Rigidbody body)
{
    return 0.0f;
}

void physics_body_mass_set(Rigidbody body, float mass)
{
    /*dMass dmass;
      dMassAdjust(&dmass, mass);
      dBodySetMass(body, &dmass);*/
}

void physics_body_data_set(Rigidbody body, void * data)
{
    dBodySetData(body, data);
}

void * physics_body_data_get(Rigidbody body)
{
    return dBodyGetData(body);
}

void physics_body_kinematic_set(Rigidbody body)
{
    dBodySetKinematic(body);
}

void physics_cs_ray_hit_callback(void* data, dGeomID geom1, dGeomID geom2)
{
    dContact contact;
    if(dCollide(geom1, geom2, 1, &contact.geom, sizeof(contact)) == 1)
    {
	struct Raycast_Hit* rayhit = (struct Raycast_Hit*)data;
	Rigidbody body = dGeomGetBody(geom2);
	if(body)
	{
	    int entity_id = (int)dBodyGetData(body);
	    if(entity_id != 0) rayhit->entity_id = entity_id;
	}
	else
	{
	    int entity_id = (int)dGeomGetData(geom2);
	    if(entity_id != 0) rayhit->entity_id = entity_id;
	}
    }
}

bool physics_cs_ray_cast(Collision_Shape ray, 
			 struct Raycast_Hit* out_rayhit, 
			 float pos_x, float pos_y, float pos_z, 
			 float dir_x, float dir_y, float dir_z)
{
    assert(out_rayhit);
    out_rayhit->entity_id = -1;
    dGeomRaySet(ray, pos_x, pos_y, pos_z, dir_x, dir_y, dir_z);
    dSpaceCollide2(ray, Physics.space, (void*)out_rayhit, &physics_cs_ray_hit_callback);
    return out_rayhit->entity_id == -1 ? false : true;
}

void physics_cs_position_get(Collision_Shape shape, float * x, float * y, float * z)
{
    assert(x && y && z);
    if(dGeomGetClass(shape) == dPlaneClass)
    {
	*x = 0.f;
	*y = 0.f;
	*z = 0.f;
	return;
    }

    const dReal* pos = dGeomGetPosition(shape);
    *x = pos[0];
    *y = pos[1];
    *z = pos[2];
}

void physics_cs_position_set(Collision_Shape shape, float x, float y, float z)
{
    if(dGeomGetClass(shape) == dPlaneClass)
	return;

    dGeomSetPosition(shape, x, y, z);
}

void physics_cs_rotation_get(Collision_Shape shape, float* x, float* y, float* z, float* w)
{
    assert(x && y && z && w);

    if(dGeomGetClass(shape) == dPlaneClass)
    {
	*x = 0.f;
	*y = 0.f;
	*z = 0.f;
	*w = 1.f;
	return;
    }

    dReal rotation[4] = { 1, 0, 0, 0 };
    dGeomGetQuaternion(shape, &rotation[0]);
    *x = rotation[1];
    *y = rotation[2];
    *z = rotation[3];
    *w = rotation[0];
}

void physics_cs_rotation_set(Collision_Shape shape, float x, float y, float z, float w)
{
    if(dGeomGetClass(shape) == dPlaneClass)
	return;

    dReal rotation[4] = { 0.f };
    rotation[1] = x;
    rotation[2] = y;
    rotation[3] = z;
    rotation[0] = w;
    dGeomSetQuaternion(shape, &rotation[0]);
}

Collision_Shape physics_cs_ray_create(float length, bool first_contact, bool backface_cull)
{
    dGeomID ray = dCreateRay(Physics.space, length);
    dGeomRaySetParams(ray, first_contact, backface_cull);
    return ray;
}

void physics_body_remove(Rigidbody body)
{
    dGeomID shape = dBodyGetFirstGeom(body);
    if(shape)
	physics_cs_remove(shape);
    dBodyDestroy(body);
}

void physics_cs_remove(Collision_Shape shape)
{
    dGeomDestroy(shape);
}

void physics_cs_data_set(Collision_Shape shape, void* data)
{
    assert(shape);
    dGeomSetData(shape, data);
}

void* physics_cs_data_get(Collision_Shape shape)
{
    assert(shape);
    return dGeomGetData(shape);
}
