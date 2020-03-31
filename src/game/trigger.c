#include "trigger.h"
#include "entity.h"
#include "game.h"
#include "event.h"
#include "bounding_volumes.h"
#include "scene.h"

#include <assert.h>

void trigger_init(struct Trigger* trigger, int type, int trigger_mask)
{
	assert(type < TRIG_MAX);

	trigger->base.type = ET_TRIGGER;
	trigger->count = 0;
	trigger->triggered = false;
	trigger->type = type;
	trigger->trigger_mask = trigger_mask;
}

void trigger_reset(struct Trigger* trigger)
{

}

void trigger_update_physics(struct Trigger* trigger, struct Scene* scene, float fixed_dt)
{
	// Check if we're triggered and fire the associated event
	bool intersecting = false;
	if(trigger->trigger_mask & TRIGM_PLAYER)
	{
		int intersection = bv_intersect_bounding_boxes(&trigger->base.derived_bounding_box, &scene->player.base.derived_bounding_box);
		if(intersection == IT_INSIDE || intersection == IT_INTERSECT)
		{
			intersecting = true;
		}
	}

	if(trigger->trigger_mask & TRIGM_ENEMY)
	{
		for(int i = 0; i < MAX_SCENE_ENEMIES; i++)
		{
			struct Enemy* enemy = &scene->enemies[i];
			if(!(enemy->base.flags & EF_ACTIVE))
				continue;

			int intersection = bv_intersect_bounding_boxes(&trigger->base.derived_bounding_box, &enemy->mesh->base.derived_bounding_box);
			if(intersection == IT_INTERSECT || intersection == IT_INSIDE)
			{
				intersecting = true;
			}
		}
	}

	if(intersecting)
	{
		bool fire_event = false;
		switch(trigger->type)
		{
		case TRIG_ONE_SHOT:
		{
			fire_event = true;
			trigger->triggered = true;
			trigger->count++;
		}
		break;
		case TRIG_TOGGLE:
		{
			// if trigger is already triggered last frame and is also intersecting this frame, then 
			// it means event has already been fired and we cannot fire an event until it is empty
			if(trigger->triggered)
			{
				fire_event = false;
			}
			else
			{
				trigger->triggered = true;
				trigger->count++;
				fire_event = true;
			}
		}
		break;
		case TRIG_CONTINUOUS:
		{
			trigger->triggered = true;
			trigger->count++;
			fire_event = true;
		}
		break;
		}

		if(fire_event)
		{
			struct Event_Manager* event_manager = game_state_get()->event_manager;
			struct Event* trigger_event = event_manager_create_new_event(event_manager);
			trigger_event->type = EVT_TRIGGER;
			trigger_event->trigger.sender = trigger;
			trigger_event->sender = trigger;
			event_manager_send_event(event_manager, trigger_event);
			if(trigger->type == TRIG_ONE_SHOT)
				scene_trigger_remove(scene, trigger);
		}
	}
	else
	{
		trigger->triggered = false;
	}
}
