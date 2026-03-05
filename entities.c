#include "vector2int.h"

enum EntityType {
	NONE,
	PLAYER,
	WALL,
	ENEMY,
	TORCH,
};

typedef struct {
	char* name;
	enum EntityType type;
	Vector2Int position;
	int hp;
	int damage;
	int inverse_speed;
	int impetus_to_move; // When this hits inverse_speed, the player can move one tile->
	int attack_delay;
	int impetus_to_attack; // When this hits attack_delay, the player can move one tile->
	bool blocking; // Can you pathfind through this?
	bool bumpable; // If you bump this, should you attack it?
	int vision; // How many tiles away can this see?
	int aggro_entity_id;
} Entity; 

// The following functions are helper functions for creating entities:

void Position(Entity* e, int x, int y) {
	e->position.x = x;
	e->position.y = y;
}

void Combat(Entity* e, int hp, int damage, int attack_delay) {
	e->hp = hp;
	e->damage = damage;
	e->attack_delay = attack_delay;
	e->bumpable = 1;
	e->blocking = 1;
}

void Movement(Entity* e, int inverse_speed) {
	e->inverse_speed = inverse_speed;
}

void Player(Entity* e) {
	e->type = PLAYER;
	e->name = "Dormin";
	Combat(e, 10, 5, 10);
	Movement(e, 10);
}

void Wall(Entity* e, int x, int y) {
	e->name = "Wall";
	e->type = WALL;
	e->blocking = 1;
	e->hp = 1000;
	Position(e, x, y);
}

void Goblin(Entity* e, int x, int y) {
	e->name = "Goblin";
	e->type = ENEMY;
	Position(e, x, y);
	Combat(e, 15, 2, 6);
	Movement(e, 12);
}
