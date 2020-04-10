#ifndef PLAYER_H
#define PLAYER_H

struct Player;
struct Scene;
struct Pickup;

void player_init(struct Player* player, struct Scene* scene);
void player_destroy(struct Player* player);
void player_apply_damage(struct Player* player, struct Enemy* enemy);
void player_on_pickup(struct Player* player, struct Pickup* pickup);
void player_update_physics(struct Player* player, struct Scene* scene, float dt);
void player_update(struct Player* player, float dt);

#endif
