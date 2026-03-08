#ifndef ENTITIES_C
#define ENTITIES_C
#include "vector2int.h"
#include "colors.h"
#include <stdbool.h>
#include <stddef.h>
#include <ncurses.h>

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
	int map;

	int explored; // Has the player seen this tile before?
	int is_static; // Whether this should update when it's not in the player's line of sight.
	char avatar;
	enum GameColors color_pair;
	int attributes; // Combine attributes like A_BOLD || A_UNDERLINE
} Entity; 

// The following functions are helper functions for instantiating entities:

Entity* Position(Entity* e, int x, int y) {
	e->position.x = x;
	e->position.y = y;
	return e;
}

Entity* Combat(Entity* e, int hp, int damage, int attack_delay) {
	e->hp = hp;
	e->damage = damage;
	e->attack_delay = attack_delay;
	e->bumpable = 1;
	e->vision = 8;
	return e;
}

Entity* Movement(Entity* e, int inverse_speed) {
	e->inverse_speed = inverse_speed;
	return e;
}

Entity* Player(Entity* e) {
	e->type = PLAYER;
	e->name = "Dormin";
	e->avatar = '@';
	e->color_pair = GC_player;
	Combat(e, 10, 5, 10);
	Movement(e, 10);
	return e;
}

Entity* Wall(Entity* e) {
	e->name = "Wall";
	e->type = WALL;
	e->avatar = '#';
	e->blocking = 1;
	e->hp = 1000;
	e->is_static = 1;
	e->color_pair = GC_wall;
	return e;
}

Entity* Goblin(Entity* e) {
	e->name = "Goblin";
	e->type = ENEMY;
	e->avatar = 'g';
	e->color_pair = GC_goblin;
	Combat(e, 15, 1, 15);
	Movement(e, 12);
	return e;
}

Entity* Item(Entity *e) {
	e->type = ITEM;
	e->is_static = 1;
	return e;
}

Entity* Arrows(Entity* e, int amount) {
	Item(e);
	e->name = "a bundle of arrows";
	e->avatar = '&';
	e->arrows = amount;
	e->color_pair = GC_arrows;
	return e;
}

Entity* SingleArrow(Entity* e) {
	Arrows(e, 1);
	e->name = "an arrow";
	e->avatar = '1';
	return e;
}

Entity* Armor(Entity* e, int amount) {
	Item(e);
	e->name = "an armor shard";
	e->avatar = '\'';
	e->color_pair = GC_armor;
	e->armor = amount;
	return e;
}

Entity* Health(Entity* e, int amount) {
	Item(e);
	e->name = "a health potion";
	e->avatar = '+';
	e->color_pair = GC_health;
	e->attributes = A_BOLD;
	e->hp = amount;
	return e;
}

Entity* Map(Entity* e, int level_number) {
	Item(e);
	e->name = "a map of the floor";
	e->avatar = 'M';
	e->color_pair = GC_map;
	e->attributes = A_BOLD | A_UNDERLINE;
	e->map = level_number;
	return e;
}

Entity* Gold(Entity* e, int amount) {
	Item(e);
	e->name = "a bag of gold";
	e->avatar = '$';
	e->color_pair = GC_gold;
	e->gold = amount;
	return e;
}

Entity* SmallGold(Entity* e) {
	Gold(e, 3);
	e->name = "a small handful of coins";
	return e;
}

Entity* MediumGold(Entity* e) {
	Gold(e, 10);
	e->name = "a nice chunk of change";
	return e;
}

Entity* LargeGold(Entity* e) {
	Gold(e, 18);
	e->name = "a massive wad of wonga!";
	return e;
}

Entity* Staircase(Entity* e){
	e->type = STAIRCASE;
	e->avatar = '>';
	e->is_static = 1;
	e->color_pair = GC_staircase;
	return e;
}

#endif
