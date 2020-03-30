#include "scene_funcs.h"
#include "event.h"
#include "game.h"

#include "../common/log.h"

static void scene_on_end_trigger(const struct Event* event, void* sender);

void scene_init_stub(struct Scene* scene)
{
	log_warning("Scene Init Stub Called");
}

void scene_cleanup_stub(struct Scene* scene)
{
	log_warning("Scene Cleanup Stub Called");
}

void scene_1_init(struct Scene* scene)
{
	struct Trigger* scene_end_trigger = scene_trigger_find(scene, "Scene_End_Trigger");
	if(scene_end_trigger)
	{
		struct Event_Manager* event_manager = game_state_get()->event_manager;
		event_manager_subscribe_with_sender(event_manager, EVT_TRIGGER, &scene_on_end_trigger, scene_end_trigger);
		log_message("Scene End Event set");
	}
	else
	{
		log_message("Trigger not found");
	}
}

void scene_1_cleanup(struct Scene* scene)
{
	struct Trigger* scene_end_trigger = scene_trigger_find(scene, "Scene_End_Trigger");
	if(scene_end_trigger)
	{
		struct Event_Manager* event_manager = game_state_get()->event_manager;
		event_manager_unsubscribe_sender(event_manager, EVT_TRIGGER, &scene_on_end_trigger, scene_end_trigger);
	}
}

void scene_on_end_trigger(const struct Event* event, void* sender)
{
	log_message("Scene_End_Trigger triggered, Move to next scene now!");
}
