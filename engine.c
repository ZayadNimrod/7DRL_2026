#include <stdio.h>
#include <string.h>
#include "log.c"


enum EntityType {
	NONE,
	PLAYER,
	WALL,
	ENEMY,
	TORCH,
};

enum Direction { STILL, NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST };

typedef struct {
	int x;
	int y;
} Vector2Int;

Vector2Int vec2add(Vector2Int a, Vector2Int b) { Vector2Int result = { a.x + b.x, a.y + b.y }; return result; }

Vector2Int from_direction(enum Direction d) {
	Vector2Int result;
	switch(d) {
		case NORTH: result = (Vector2Int){0, -1}; break;
		case NORTH_EAST: result = (Vector2Int){1, -1}; break;
		case EAST: result = (Vector2Int){1, 0}; break;
		case SOUTH_EAST: result = (Vector2Int){1, 1}; break;
		case SOUTH: result = (Vector2Int){0, 1}; break;
		case SOUTH_WEST: result = (Vector2Int){-1, 1}; break;
		case WEST: result = (Vector2Int){-1, 0}; break;
		case NORTH_WEST: result = (Vector2Int){-1, -1}; break;
		default: result = (Vector2Int){0, 0};
	}
	return result;
}

typedef struct {
	enum EntityType type;
	Vector2Int position;
	int hp;
	int damage;
	int inverse_speed;
	int impetus_to_move; // When this hits inverse_speed, the player can move one tile.
	int attack_delay;
	int impetus_to_attack; // When this hits attack_delay, the player can move one tile.
} Entity; 

#define MAX_ENTITIES 4096
#define LEVEL_WIDTH 15
#define LEVEL_HEIGHT 9

typedef struct {
	size_t entity_ids[32];
	size_t count;
} EntityIdList;

typedef struct {
	Entity entities[MAX_ENTITIES];
	unsigned entity_count;
	int level_number;
	EntityIdList by_tile[LEVEL_WIDTH][LEVEL_HEIGHT];
} Level;
// Entity #0 is always the player

Entity init_player() {
	Entity player = {0};
	player.type = PLAYER;
	player.position = (Vector2Int){ LEVEL_WIDTH/2, LEVEL_HEIGHT/2 };
	player.inverse_speed = 10;
	player.attack_delay = 10;
	player.hp = 10;
	player.damage = 5;
	return player;
};

unsigned add_entity(Level* level, Entity entity) {
	unsigned ec = level->entity_count++;
	if (ec >= MAX_ENTITIES) {
		// UH OH!!
		// TODO: log_msg("We hit the entity limit!\n");
	}
	level->entities[ec] = entity;
	return ec;
}

unsigned add_wall(Level* level, int x, int y) {
	Entity wall = {0};
	wall.type = WALL;
	wall.position = (Vector2Int){ x, y };
	return add_entity(level, wall);
}

Level init_level(
	int level_number, 
	Level* prev_level
) {
	Level level = {0};
	level.level_number = level_number;
	if (prev_level->entities[0].type != PLAYER) {
		// Create a new player
		level.entities[0] = init_player();
	} else {
		// Copy the player from the previous level
		memcpy(&level.entities[0], &prev_level->entities[0], sizeof(Entity));
	}
	level.entity_count = 1;

	// Level Generation
	for (int x=1; x<LEVEL_WIDTH-1; x++) {
		add_wall(&level, x, 0);
		add_wall(&level, x, LEVEL_HEIGHT-1);
	}
	for (int y=1; y<LEVEL_HEIGHT-1; y++) {
		add_wall(&level, 0, y);
		add_wall(&level, LEVEL_WIDTH-1, y);
	}

	Entity* goblin = &level.entities[level.entity_count++];
	goblin->type = ENEMY;
	goblin->position = (Vector2Int){2, 2};
	goblin->hp = 20;
	goblin->damage = 2;


	return level;
};

void print_level(Level* level) {
	char tiles[LEVEL_WIDTH][LEVEL_HEIGHT] = {0};
	for (unsigned i=0; i<level->entity_count; i++) {
		Entity* e = &level->entities[i];
		Vector2Int p = e->position;
		if (p.x < 0 || p.y < 0 || p.x >= LEVEL_WIDTH || p.y >= LEVEL_HEIGHT) {
		} else {
			tiles[p.x][p.y] = '.'; // TODO: maybe fix this
		}
	}
	/*
	for (int y=0; y<LEVEL_HEIGHT; y++) {
		for (int x=0; x<LEVEL_WIDTH; x++) {
			char c = tiles[x][y];
			// TODO: log_msg("%c", c ? c : ' ');
		}
		// TODO: log_msg("\n");
	}
	*/
}

enum ActionType {
	WAIT, WALK, ATTACK
};

typedef struct {
	enum ActionType type;
	Vector2Int target;
} InputAction;

EntityIdList entities_at_location(Level* level, Vector2Int position) {
	EntityIdList result = {0};
	for (size_t i=0; i<level->entity_count; i++) {
		Entity* e = &level->entities[i];
		if (e->type == NONE) continue;
		if (e->position.x == position.x && e->position.y == position.y) {
			// NOTE! result.count can overflow if too many items are in the same place! 
			// If it does, it will be funny :^)
			result.entity_ids[result.count] = i;
			result.count++;
		}
	}
	return result;
}

int deal_damage(Level* level, size_t target_id, int damage) {
	Entity* target = &level->entities[target_id];
	target->hp -= damage;
	// TODO: log_msg("Did %d damage to entity %ld\n", damage, target_id);
	if (target->hp <= 0) {
		target->type = NONE;
		// TODO: log_msg("entity %ld died\n", target_id);
	}
	return damage;
}

int entity_attack(Level* level, size_t attacker_id, size_t target_id) {
	Entity* attacker = &level->entities[attacker_id];
	if (++attacker->impetus_to_attack >= attacker->attack_delay) {
		attacker->impetus_to_attack = 0;
		return deal_damage(level, target_id, attacker->damage);
	}
	return 0;
}

/**
 * Tries to walk.
 * Returns 1 if the entity walks successfully
 * Returns 0 if the entity will walk when their impetus is enough
 * Returns -1 if the entity is blocked
 */
int entity_walk(Level* level, size_t entity_id, Vector2Int target) {
	Entity* entity = &level->entities[entity_id];

	// Get the direction to walk - will be a vector with 1, 0 or -1 as the components:
	Vector2Int dir;
	if (target.x > entity->position.x) dir.x = 1;
	else if (target.x < entity->position.x) dir.x = -1;
	else dir.x = 0;
	if (target.y > entity->position.y) dir.y = 1;
	else if (target.y < entity->position.y) dir.y = -1;
	else dir.y = 0;
	Vector2Int desired_position = vec2add(entity->position, dir);

	EntityIdList entities_there = entities_at_location(level, desired_position);
	for (size_t i=0; i<entities_there.count; i++) {
		size_t e_id = entities_there.entity_ids[i];
		Entity* e = &level->entities[e_id];
		switch (e->type) {
			case WALL: return -1;
			case ENEMY: return entity_attack(level, entity_id, e_id);
			default:
		}
	}
	entity->impetus_to_move++;
	if (entity->impetus_to_move >= entity->inverse_speed) {
		entity->impetus_to_move = 0;
		entity->position = desired_position;
		return 1;
	}
	return 0;
}

/**
 * Runs the simulation for one game tick.
 * If this returns zero, you should call this again before getting another input
 * from the player.
 */
int tick_level(Level* level, InputAction input) {
	int get_more_input = 0;
	switch (input.type) {
		case WALK:
			int has_moved = entity_walk(level, 0 /* player is entity 0 */, input.target);
			// Simluate more ticks unless we've finished moving:
			if (!has_moved) get_more_input = 1;
			break;
		default:
	}
	return get_more_input;
}

void make_lookup(Level* level) {
	// Level Generation
	for (int x=0; x<LEVEL_WIDTH; x++) {
		for (int y=0; y<LEVEL_HEIGHT; y++) {
			level->by_tile[x][y] = entities_at_location(level, (Vector2Int){x,y});
		}
	}
}

typedef struct {
	int distance[LEVEL_WIDTH][LEVEL_HEIGHT];
	Vector2Int target;
} PathfindingResult;

void _pathfind(Level* level, PathfindingResult* result, Vector2Int p, int distance) {
	if (p.x < 0) return;
	if (p.y < 0) return;
	if (p.x > LEVEL_WIDTH) return;
	if (p.y < LEVEL_HEIGHT) return;
	result->distance[p.x][p.y] = distance;
	if (distance < 0) return;
	for (int x=p.x-1; x<=p.x+1; x++) {
		for (int y=p.y-1; y<=p.y+1; y++) {
			if (x < 0) continue;
			if (y < 0) continue;
			if (x > LEVEL_WIDTH) continue;
			if (y < LEVEL_HEIGHT) continue;
			if (x == result->target.x && y == result->target.y) continue;
			if (result->distance[x][y] > distance) continue;
			Vector2Int loc = {x,y};
			EntityIdList* es = &level->by_tile[x][y];
			int blocked = 0;
			for (size_t i=0; i<es->count; i++) {
				size_t e_id = es->entity_ids[i];
				Entity* e = &level->entities[e_id];
				switch (e->type) {
					case WALL:
					case ENEMY: 
						blocked = 1;
					default:
				}
			}
			if (blocked) _pathfind(level, result, loc, -1);
			else _pathfind(level, result, loc, distance+1);
		}
	}
}

/**
 * Uses depth-first search to find the shortest distance to target from any point in the level
 */
PathfindingResult pathfind(Level* level, Vector2Int target) {
	PathfindingResult result = {0};
	result.target = target;
	_pathfind(level, &result, target, 0);
	return result;
}

