#include "physics.h"
#include "../common/log.h"

#include <Newton.h>

static NewtonWorld* newton_world = 0;

void physics_init(void)
{
	newton_world = NewtonCreate();
	if(!newton_world)
	{
		log_error("physics:init", "Physics world created!");
	}
	else
	{
		log_message("Physics world created");
	}
}

void physics_cleanup(void)
{
	if(newton_world)
	{
		NewtonDestroy(newton_world);
	}
}
