#ifndef PLAYER_H
#define PLAYER_H

struct Player;
struct Scene;

void player_init(struct Player* player, struct Scene* scene);
void player_destroy(struct Player* player);
void player_apply_damage(struct Player* player, struct Enemy* enemy);
void player_update_physics(struct Player* player, struct Scene* scene, float dt);

#endif
