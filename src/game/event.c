#include "event.h"
#include "../common/log.h"

#include <string.h>
#include <assert.h>

#include <SDL.h>

void event_manager_init(struct Event_Manager* event_manager)
{
	assert(event_manager);

	for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
	{
		struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
		memset(subscription, 0, sizeof(struct Event_Subscription));
		subscription->event_type = EVT_NONE;
		subscription->type = EST_NONE;
	}

	for(int i = 0; i < MAX_EVENTS; i++)
	{
		struct Event* event = &event_manager->event_pool[i];
		memset(event, '\0', sizeof(struct Event));
		event->type = EVT_NONE;
	}

	event_manager->sdl_event_id = SDL_RegisterEvents(1);
}

void event_manager_subscribe(struct Event_Manager* event_manager, int event_type, Event_Handler handler_func)
{
	assert(event_manager && event_type < EVT_MAX && event_type > EVT_NONE);

	//Check if this handler/subscriber already exists
	bool subscribed = false;
	for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
	{
		struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
		if(subscription->type == EST_WITHOUT_OBJECT && subscription->event_type == event_type && subscription->handler == handler_func)
		{
			log_message("Already subscibed to %s event", event_name_get(event_type));
			subscribed = true;
			break;
		}
	}

	//Now that we've established that we are not subscribed already we find an empty slot and 
	//create a new subscription there
	if(!subscribed)
	{
		for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
		{
			struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
			if(subscription->type == EST_NONE)
			{
				subscription->type       = EST_WITHOUT_OBJECT;
				subscription->event_type = event_type;
				subscription->handler    = handler_func;
				subscribed               = true;
				break;
			}
		}
	}

	//if subscribed is still not true, it can only mean that all the existing slots are taken
	//Show an error message in that case
	if(!subscribed)
		log_error("event_manager:subscribe", "Could not subscribe to %s event", event_name_get(event_type));
}

void event_manager_unsubscribe(struct Event_Manager* event_manager, int event_type, Event_Handler handler_func)
{
	assert(event_manager && event_type < EVT_MAX);

	for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
	{
		struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
		if(subscription->type != EST_WITHOUT_OBJECT) continue;
		if(subscription->event_type == event_type && subscription->handler == handler_func)
		{
			subscription->handler    = NULL;
			subscription->event_type = EVT_NONE;
			subscription->type       = EST_NONE;
			break;
		}
	}
}

void event_manager_unsubscribe_with_object(struct Event_Manager* event_manager, int event_type, Event_Handler_Object handler_func, void* subscriber)
{
	assert(event_manager && event_type < EVT_MAX);

	for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
	{
		struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
		if(subscription->type != EST_WITH_OBJECT) continue;
		if(subscription->event_type == event_type && subscription->handler_with_object == handler_func && subscription->subscriber == subscriber)
		{
			subscription->handler_with_object = NULL;
			subscription->event_type          = EVT_NONE;
			subscription->type                = EST_NONE;
			subscription->subscriber          = NULL;
			break;
		}
	}
}

void event_manager_send_event(struct Event_Manager* event_manager, struct Event* event)
{
	assert(event_manager && event);

	SDL_Event sdl_event;
	SDL_memset(&sdl_event, 0, sizeof(sdl_event));
	sdl_event.type = event_manager->sdl_event_id;
	sdl_event.user.data1 = event;
	int rc = SDL_PushEvent(&sdl_event);
	if(rc != 1)
	{
		log_error("event_manager:send_event", "Could not push event, %s", event_name_get(event->type));
	}
}

struct Event* event_manager_create_new_event(struct Event_Manager* event_manager)
{
	struct Event* new_event = NULL;
	for(int i = 0; i < MAX_EVENTS; i++)
	{
		if(event_manager->event_pool[i].type == EVT_NONE)
		{
			new_event = &event_manager->event_pool[i];
			break;
		}
	}

	if(!new_event)
		log_warning("Out of event slots, event pool full!");

	return new_event;
}

void event_manager_poll_events(struct Event_Manager* event_manager, bool* out_quit)
{
	static SDL_Event event;
	while(SDL_PollEvent(&event) != 0)
	{
		switch(event.type)
		{
		case SDL_QUIT:
			*out_quit = true;
			break;
		case SDL_KEYDOWN: case SDL_KEYUP:
		{
			int  scancode  = event.key.keysym.scancode;
			int  key       = event.key.keysym.sym;
			int  state     = event.key.state;
			bool repeat    = event.key.repeat;
			bool mod_ctrl  = (event.key.keysym.mod & KMOD_CTRL) ? true : false;
			bool mod_shift = (event.key.keysym.mod & KMOD_SHIFT) ? true : false;
			bool mod_alt   = (event.key.keysym.mod & KMOD_ALT) ? true : false;

			struct Event* new_event = event_manager_create_new_event(event_manager);
			new_event->type = event.type == SDL_KEYDOWN ? EVT_KEY_PRESSED : EVT_KEY_RELEASED;
			new_event->key.key       = event.key.keysym.sym;
			new_event->key.scancode  = event.key.keysym.scancode;
			new_event->key.state     = event.key.state;
			new_event->key.repeat    = event.key.repeat == 0 ? false : true;
			new_event->key.mod_ctrl  = (event.key.keysym.mod & KMOD_CTRL) ? true : false;
			new_event->key.mod_shift = (event.key.keysym.mod & KMOD_SHIFT) ? true : false;
			new_event->key.mod_alt   = (event.key.keysym.mod & KMOD_ALT) ? true : false;
			event_manager_send_event(event_manager, new_event);
			//log_message("Key name : %s", SDL_GetKeyName(key));
			break;
		}
		case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP:
		{
			struct Event* new_event = event_manager_create_new_event(event_manager);
			new_event->type = event.type == SDL_MOUSEBUTTONDOWN ? EVT_MOUSEBUTTON_PRESSED : EVT_MOUSEBUTTON_RELEASED;
			new_event->mousebutton.button     = event.button.button;
			new_event->mousebutton.state      = event.button.state;
			new_event->mousebutton.num_clicks = event.button.clicks;
			new_event->mousebutton.x          = event.button.x;
			new_event->mousebutton.y          = event.button.y;
			event_manager_send_event(event_manager, new_event);
			break;
		}
		case SDL_MOUSEMOTION:
		{
			struct Event* new_event = event_manager_create_new_event(event_manager);
			new_event->type = EVT_MOUSEMOTION;
			new_event->mousemotion.xrel = event.motion.xrel;
			new_event->mousemotion.yrel = event.motion.yrel;
			new_event->mousemotion.x    = event.motion.x;
			new_event->mousemotion.y    = event.motion.y;
			event_manager_send_event(event_manager, new_event);
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			struct Event* new_event = event_manager_create_new_event(event_manager);
			new_event->type = EVT_MOUSEWHEEL;
			new_event->mousewheel.x = event.wheel.x;
			new_event->mousewheel.y = event.wheel.y;
			event_manager_send_event(event_manager, new_event);
			break;
		}
		case SDL_TEXTINPUT:
		{
			struct Event* new_event = event_manager_create_new_event(event_manager);
			new_event->type = EVT_TEXT_INPUT;
			memcpy(new_event->text_input.text, event.text.text, 32);
			event_manager_send_event(event_manager, new_event);
			break;
		}
		case SDL_WINDOWEVENT:
		{
			if(event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				struct Event* new_event = event_manager_create_new_event(event_manager);
				new_event->type = EVT_WINDOW_RESIZED;
				new_event->window_resize.width = event.window.data1;
				new_event->window_resize.height = event.window.data2;
				event_manager_send_event(event_manager, new_event);
			}
		}
		break;
		default:
		{
			if(event.type == event_manager->sdl_event_id)
			{
				struct Event* user_event = (struct Event*)event.user.data1;
				//log_message("%s event", event_name_get(user_event->type));
				for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
				{
					struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
					if(subscription->type == EST_NONE) continue;

					if(subscription->event_type == user_event->type)
					{
						if(subscription->type == EST_WITHOUT_OBJECT)
						{
							if(subscription->handler) 
								subscription->handler(user_event);
						}
						else if(subscription->type == EST_WITH_OBJECT)
						{
							if(subscription->handler_with_object)
								subscription->handler_with_object(user_event, subscription->subscriber);
						}
					}
				}

				//return event to the pool now that it is consumed
				memset(user_event, '\0', sizeof(*user_event));
				user_event->type = EVT_NONE;
			}
		}
		break;
		}
	}
}

void event_manager_subscribe_with_object(struct Event_Manager* event_manager, int event_type, Event_Handler_Object handler_func, void* subscriber)
{
	assert(event_manager && event_type < EVT_MAX && event_type > EVT_NONE);

	//Check if this handler/subscriber already exists
	bool subscribed = false;
	for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
	{
		struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
		if(subscription->type == EST_WITH_OBJECT && subscription->event_type == event_type && subscription->handler_with_object == handler_func && subscription->subscriber != subscriber)
		{
			log_message("Already subscibed to %s event", event_name_get(event_type));
			subscribed = true;
			break;
		}
	}

	//Now that we've established that we are not subscribed already we find an empty slot and 
	//create a new subscription there
	if(!subscribed)
	{
		for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
		{
			struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
			if(subscription->type == EST_NONE)
			{
				subscription->type                = EST_WITH_OBJECT;
				subscription->event_type          = event_type;
				subscription->handler_with_object = handler_func;
				subscription->subscriber          = subscriber;
				subscribed                        = true;
				break;
			}
		}
	}

	//if subscribed is still not true, it can only mean that all the existing slots are taken
	//Show an error message in that case
	if(!subscribed)
		log_error("event_manager:subscribe_with_obejct", "Could not subscribe to %s event", event_name_get(event_type));
	
}

void event_manager_cleanup(struct Event_Manager* event_manager)
{

}

const char* event_name_get(int event_type)
{
	assert(event_type >= EVT_NONE && event_type <= EVT_MAX);

	switch(event_type)
	{
	case EVT_NONE:                 return "None";
	case EVT_KEY_PRESSED:          return "Key Pressed";
	case EVT_KEY_RELEASED:         return "Key Released";
	case EVT_MOUSEBUTTON_PRESSED:  return "Mousebutton Pressed";
	case EVT_MOUSEBUTTON_RELEASED: return "Mousebutton Released";
	case EVT_MOUSEMOTION:          return "Mouse Motion";
	case EVT_MOUSEWHEEL:           return "Mouse Wheel";
	case EVT_WINDOW_RESIZED:       return "Window Resized";
	case EVT_TEXT_INPUT:           return "Text Input";
	case EVT_SCENE_LOADED:         return "Scene Loaded";
	case EVT_MAX:                  return "Max Number of Events";
	default: return "Invalid event_type";
	}
}

void event_manager_send_event_entity(struct Event_Manager* event_manager, struct Event* event, struct Entity* entity)
{
	// Check if this entity has a subscription, if it does,
	// call the registered callback with this entity as the parameter to simulate an event
	for(int i = 0; i < MAX_EVENT_SUBSCRIPTIONS; i++)
	{
		struct Event_Subscription* subscription = &event_manager->event_subsciptions[i];
		if(subscription->type != EST_WITH_OBJECT) continue;

		if(subscription->event_type == event->type && subscription->subscriber == entity && subscription->handler_with_object)
		{
			subscription->handler_with_object(event, (void*)entity);
			break;
		}
	}
}
