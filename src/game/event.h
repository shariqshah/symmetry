#ifndef EVENT_H
#define EVENT_H

#include "../common/linmath.h"
#include "../common/num_types.h"

typedef void (*Event_Handler) (const struct Event* event);

#define MAX_EVENTS 128
#define MAX_EVENT_SUBSCRIPTIONS 256

enum Event_Types
{
	EVT_NONE = 0,
	EVT_KEY_PRESSED,
	EVT_KEY_RELEASED,
	EVT_MOUSEBUTTON_PRESSED,
	EVT_MOUSEBUTTON_RELEASED,
	EVT_MOUSEMOTION,
	EVT_WINDOW_RESIZED,
	EVT_MAX
};

struct Key_Event
{
	int  scancode;
	int  key;
	int  state;
	bool repeat;
	bool mod_ctrl;
	bool mod_shift;
	bool mod_alt;
};

struct Player_Damage_Event
{
	int  damage;
	int  enemy;
	vec3 direction;
};

struct Event
{
	int type;
	union
	{
		struct Player_Damage_Event player_damage;
		struct Key_Event           key;
	};
};

struct Event_Subscription
{
	int           event_type;
	Event_Handler handler;
};

struct Event_Manager
{
	struct Event              event_pool[MAX_EVENTS];
	struct Event_Subscription event_subsciptions[MAX_EVENT_SUBSCRIPTIONS];
	uint32                    sdl_event_id;
};

////Event subsciption example
//void player_init()
//{
//	struct Event_Manager* event_manager = game_state_get()->event_manager;
//	event_manager_subscribe(event_manager, EVT_PLAYER_DAMAGE, &on_player_damage);
//}
//
////Event unsubscribe example
//void player_cleanup()
//{
//	struct Event_Manager* event_manager = game_state_get()->event_manager;
//	event_manager_unsubscribe(event_manager, EVT_PLAYER_DAMANGE, &on_player_damage);
//}
//
////Event recieve example usage
//void on_player_damage(struct Event* event_data)
//{
//	struct Player_Damage_Event* player_damage_event = &event_data->player_damage;
//	damage_player(player_damage_event->damage, player_damage_event->direction);
//}
//
////Event send example usage
//void enemy_tick()
//{
//	struct Event_Manager* event_manager = game_state-get()->event_manager;
//	struct Event* new_event = event_manager_create_new_event(event_manager);
//	new_event->type = EVT_PLAYER_DAMAGE;
//	new_event->player_damage.damage = 20;
//	new_event->player_damage.enemy = enemy_id;
//	event_manager_send_event(event_manager, new_event);
//}
//

void          event_manager_init(struct Event_Manager* event_manager);
void          event_manager_subscribe(struct Event_Manager* event_manager, int event_type, Event_Handler subscriber);
void          event_manager_unsubscribe(struct Event_Manager* event_manager, int event_type, Event_Handler subscriber);
struct Event* event_manager_create_new_event(struct Event_Manager* event_manager);
void          event_manager_send_event(struct Event_Manager* event_manager, struct Event* event);
void          event_manager_poll_events(struct Event_Manager* event_manager, bool* out_quit);
void          event_manager_cleanup(struct Event_Manager* event_manager);
const char*   event_name_get(int event_type);

#endif
