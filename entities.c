#ifndef ENTITIES_C
#define ENTITIES_C
#include "vector2int.h"
#include <stdbool.h>
#include <stddef.h>

enum EntityType {
	NONE,
	PLAYER,
	WALL,
	ENEMY,
	ITEM,
	STAIRCASE,
};

typedef struct {
	char* name;
	enum EntityType type;
	Vector2Int position;
	int damage;
	int inverse_speed;
	int impetus_to_move; // When this hits inverse_speed, the player can move one tile->
	int attack_delay;
	int impetus_to_attack; // When this hits attack_delay, the player can move one tile->
	bool blocking; // Can you pathfind through this?
	bool bumpable; // If you bump this, should you attack it?
	int aggro_entity_id;
	// These are things you can have in an inventory.
	// If this is an item, picking it up will add those things to your inventory.
	int hp;
	int arrows;
	int armor;
	int gold;
	int vision; // How many tiles away can this see?
	int explored; // Has the player seen this tile before?
	int is_static; // Whether this should update when it's not in the player's line of sight.
	char avatar;
	short color_pair;
	int ncurses_attributes; // Combine attributes like A_BOLD || A_UNDERLINE
} Entity; 

// The following functions are helper functions for instantiating entities:

void Position(Entity* e, int x, int y) {
	e->position.x = x;
	e->position.y = y;
}

void Combat(Entity* e, int hp, int damage, int attack_delay) {
	e->hp = hp;
	e->damage = damage;
	e->attack_delay = attack_delay;
	e->bumpable = 1;
	e->vision = 8;
}

void Movement(Entity* e, int inverse_speed) {
	e->inverse_speed = inverse_speed;
}

void Player(Entity* e) {
	e->type = PLAYER;
	e->name = "Dormin";
	e->avatar = '@';
	Combat(e, 10, 5, 10);
	Movement(e, 10);
}

void Wall(Entity* e, int x, int y) {
	e->name = "Wall";
	e->type = WALL;
	e->avatar = '#';
	e->blocking = 1;
	e->hp = 1000;
	e->is_static = 1;
	Position(e, x, y);
}

void Goblin(Entity* e, int x, int y) {
	e->name = "Goblin";
	e->type = ENEMY;
	e->avatar = 'g';
	e->color_pair = 1;
	Position(e, x, y);
	Combat(e, 15, 1, 15);
	Movement(e, 12);
}

void Item(Entity *e) {
	e->type = ITEM;
	e->is_static = 1;
}

void Arrows(Entity* e, int x, int y, int amount) {
	Item(e);
	e->name = "a bundle of arrows";
	e->avatar = '&';
	Position(e, x, y);
	e->arrows = amount;
}

void Armor(Entity* e, int x, int y, int amount) {
	Item(e);
	e->name = "an armor shard";
	e->avatar = '\'';
	Position(e, x, y);
	e->armor = amount;
}

void Health(Entity* e, int x, int y, int amount) {
	Item(e);
	e->name = "a health potion";
	e->avatar = '+';
	Position(e, x, y);
	e->hp = amount;
}

void Gold(Entity* e, int x, int y, int amount) {
	Item(e);
	e->name = "a bag of gold";
	e->avatar = '$';
	Position(e, x, y);
	e->gold = amount;
}

void Staircase(Entity* e){
	e->type = STAIRCASE;
	e->avatar = '>';
}

#endif
