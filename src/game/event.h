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
	EVT_MOUSEWHEEL,
	EVT_WINDOW_RESIZED,
	EVT_TEXT_INPUT,
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

struct Event
{
	int type;
	union
	{
		struct Key_Event            key;
		struct Mousewheel_Event     mousewheel;
		struct Mousebutton_Event    mousebutton;
		struct Mousemotion_Event    mousemotion;
		struct Text_Input_Event     text_input;
		struct Window_Resized_Event window_resize;
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

void          event_manager_init(struct Event_Manager* event_manager);
void          event_manager_subscribe(struct Event_Manager* event_manager, int event_type, Event_Handler subscriber);
void          event_manager_unsubscribe(struct Event_Manager* event_manager, int event_type, Event_Handler subscriber);
struct Event* event_manager_create_new_event(struct Event_Manager* event_manager);
void          event_manager_send_event(struct Event_Manager* event_manager, struct Event* event);
void          event_manager_poll_events(struct Event_Manager* event_manager, bool* out_quit);
void          event_manager_cleanup(struct Event_Manager* event_manager);
const char*   event_name_get(int event_type);

#endif
