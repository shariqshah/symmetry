#ifndef EVENT_H
#define EVENT_H

#include "../common/linmath.h"
#include "../common/num_types.h"
#include "../common/limits.h"

struct Entity;
struct Trigger;

typedef void (*Event_Handler) (const struct Event* event);
typedef void (*Event_Handler_Sender) (const struct Event* event, void* sender);
typedef void (*Event_Handler_Subscriber) (const struct Event* event, void* subscriber);
typedef void (*Event_Handler_Subscriber_Sender) (const struct Event* event, void* subscriber, void* sender);


enum Event_Types
{
	EVT_NONE = 0,
	EVT_KEY_PRESSED,
	EVT_KEY_RELEASED,
	EVT_MOUSEBUTTON_PRESSED,
	EVT_MOUSEBUTTON_RELEASED,
	EVT_MOUSEMOTION,
	EVT_MOUSEWHEEL,
	EVT_WINDOW_RESIZED,
	EVT_TEXT_INPUT,
	EVT_SCENE_LOADED,
	EVT_TRIGGER,
	EVT_INPUT_MAP_PRESSED,
	EVT_INPUT_MAP_RELEASED,
	EVT_MAX
};

enum Event_Subscription_Type
{
	EST_NONE = 0,
	EST_WITHOUT_OBJECTS,
	EST_SENDER,
	EST_SUBSCRIBER,
	EST_SUBSCRIBER_SENDER
};

struct Input_Map_Event
{
	char name[HASH_MAX_KEY_LEN];
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

struct Mousemotion_Event
{
	int x;
	int y;
	int xrel;
	int yrel;
};

struct Mousewheel_Event
{
	int x;
	int y;
};

struct Mousebutton_Event
{
	int button;
	int state;
	int num_clicks;
	int x;
	int y;
};

struct Text_Input_Event
{
	char text[32];
};

struct Window_Resized_Event
{
	int width;
	int height;
};

struct Scene_Loaded_Event
{
	char filename[MAX_FILENAME_LEN];
};

struct Trigger_Event
{
	struct Trigger* sender;
};

struct Event
{
	int   type;
	void* sender;
	union
	{
		struct Key_Event            key;
		struct Mousewheel_Event     mousewheel;
		struct Mousebutton_Event    mousebutton;
		struct Mousemotion_Event    mousemotion;
		struct Text_Input_Event     text_input;
		struct Window_Resized_Event window_resize;
		struct Scene_Loaded_Event   scene_load;
		struct Trigger_Event        trigger;
		struct Input_Map_Event      input_map;
	};
};

struct Event_Subscription
{
	int type;
	int event_type;
	union
	{
		struct 
		{
			Event_Handler handler;
		} Subscription_Without_Objects;

		struct 
		{
			Event_Handler_Subscriber handler;
			void*                    subscriber;
		} Subscription_Subscriber;

		struct 
		{
			Event_Handler_Sender handler;
			void*                sender;
		} Subscription_Sender;

		struct 
		{
			Event_Handler_Subscriber_Sender handler;
			void*                           sender;
			void*                           subscriber;
		} Subscription_Subscriber_Sender;
	};
};

struct Event_Manager
{
	struct Event              event_pool[MAX_EVENTS];
	struct Event_Subscription event_subsciptions[MAX_EVENT_SUBSCRIPTIONS];
	uint32                    sdl_event_id;
};

void          event_manager_init(struct Event_Manager* event_manager);
void          event_manager_subscribe(struct Event_Manager* event_manager, int event_type, Event_Handler event_handler_func);
void          event_manager_subscribe_with_sender(struct Event_Manager* event_manager, int event_type, Event_Handler_Sender handler_func, void* sender);
void          event_manager_subscribe_with_subscriber(struct Event_Manager* event_manager, int event_type, Event_Handler_Subscriber handler_func, void* subscriber);
void          event_manager_subscribe_with_subscriber_sender(struct Event_Manager* event_manager, int event_type, Event_Handler_Subscriber_Sender handler_func, void* subscriber, void* sender);
void          event_manager_unsubscribe(struct Event_Manager* event_manager, int event_type, Event_Handler handler_func);
void          event_manager_unsubscribe_sender(struct Event_Manager* event_manager, int event_type, Event_Handler_Sender handler_func, void* sender);
void          event_manager_unsubscribe_with_subscriber(struct Event_Manager* event_manager, int event_type, Event_Handler_Subscriber handler_func, void* subscriber);
void          event_manager_unsubscribe_with_subscriber_sender(struct Event_Manager* event_manager, int event_type, Event_Handler_Subscriber_Sender handler_func, void* subscriber, void* sender);
struct Event* event_manager_create_new_event(struct Event_Manager* event_manager);
void          event_manager_send_event(struct Event_Manager* event_manager, struct Event* event);
void          event_manager_send_event_entity(struct Event_Manager* event_manager, struct Event* event, struct Entity* entity);
void          event_manager_poll_events(struct Event_Manager* event_manager);
void          event_manager_cleanup(struct Event_Manager* event_manager);
const char*   event_name_get(int event_type);

#endif
