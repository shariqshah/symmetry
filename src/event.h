#ifndef EVENT_H
#define EVENT_H

#include "variant.h"

typedef void (*Event_Handler) (int object_index, const struct Variant* event_params, int num_params);

struct Event_Subscription
{
	int object_index; // If a particular object has subscribed to this
					  // event then this holds the index of the object
					  // in it's particular subsystem list. For
					  // example, for a Transform, it will hold the
					  // index of the transform in the transform
					  // list. Otherwise, it holds -1.
	Event_Handler handler;
};

enum System_Event
{
	SE_KEYBOARD = 0,
	SE_MOUSEBUTTON,
	SE_MOUSEMOTION,
	SE_WINDOW_RESIZE,
	SE_NUM_EVENTS
};

#define BEGIN_EVENT_DEFINITION(event_name) 
#define END_EVENT_DEFINITION };

int  event_subscribe(int object_index, Event_Handler handler_func);
void event_unsubscribe(int subscription_index);
void event_handle_systemevent(int event_type, const struct Variant* event_params, int num_params);
void event_handle_userevent(int event_type, const struct Variant* event_params, int num_params);
void event_send(int event_type, const struct Variant* event_params, int num_params);

#endif
