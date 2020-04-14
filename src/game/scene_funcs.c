#include "scene_funcs.h"
#include "event.h"
#include "game.h"
#include "gui_game.h"

#include "../common/log.h"

static void scene_on_end_trigger(const struct Event* event, void* sender);
static void scene_on_game_end_trigger(const struct Event* event, void* sender);

void scene_func_stub(struct Scene* scene)
{
	log_warning("Scene Func Stub Called");
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
	struct Event_Manager* event_manager = game_state_get()->event_manager;
	struct Event* scene_cleared_event = event_manager_create_new_event(event_manager);
	scene_cleared_event->type = EVT_SCENE_CLEARED;
	scene_cleared_event->scene_cleared.scene = game_state_get()->scene;
	event_manager_send_event(event_manager, scene_cleared_event);
}

void scene_game_end_init(struct Scene* scene)
{
	struct Trigger* game_end_trigger = scene_trigger_find(scene, "Game_End_Trigger");
	if(game_end_trigger)
	{
		struct Event_Manager* event_manager = game_state_get()->event_manager;
		event_manager_subscribe_with_sender(event_manager, EVT_TRIGGER, &scene_on_game_end_trigger, game_end_trigger);
	}
	else
	{
		log_message("Trigger not found");
	}
}

void scene_game_end_cleanup(struct Scene* scene)
{
	struct Trigger* game_end_trigger = scene_trigger_find(scene, "Game_End_Trigger");
	if(game_end_trigger)
	{
		struct Event_Manager* event_manager = game_state_get()->event_manager;
		event_manager_unsubscribe_sender(event_manager, EVT_TRIGGER, &scene_on_game_end_trigger, game_end_trigger);
	}
}

void scene_on_game_end_trigger(const struct Event* event, void* sender)
{
	struct Event_Manager* event_manager = game_state_get()->event_manager;
	struct Event* scene_cleared_event = event_manager_create_new_event(event_manager);
	scene_cleared_event->type = EVT_SCENE_CLEARED;
	scene_cleared_event->scene_cleared.scene = game_state_get()->scene;
	event_manager_send_event(event_manager, scene_cleared_event);
	gui_game_show_game_end_dialog(game_state_get()->gui_game);
}
