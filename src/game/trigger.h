#ifndef TRIGGER_H
#define TRIGGER_H

struct Trigger;

void trigger_init(struct Trigger* trigger, int type, int trigger_event, int trigger_mask);
void trigger_reset(struct Trigger* trigger);
void trigger_update_physics(struct Trigger* trigger, struct Scene* scene, float fixed_dt);

#endif