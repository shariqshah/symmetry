#ifndef PLAYER_H
#define PLAYER_H

struct Player;

void player_init(struct Player* player, struct Scene* scene);
void player_destroy(struct Player* player);
void player_update(struct Player* player, struct Scene* scene, float dt);

#endif