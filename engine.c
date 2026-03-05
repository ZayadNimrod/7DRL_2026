#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "log.c"
#include "entities.c"
#include "vector2int.h"


enum Direction { STILL, NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST };

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
	logger_t* logger;
} Level;

Level init_level(
	int level_number, 
	Level* prev_level,
	logger_t* logger
) {
	Level level = {0};
	level.logger = logger;
	level.level_number = level_number;
	if (prev_level->entities[0].type != PLAYER) {
		// Create a new player
		Player(&level.entities[0]);
		Position(&level.entities[0], LEVEL_WIDTH/2, LEVEL_HEIGHT/2);
	} else {
		// Copy the player from the previous level
		memcpy(&level.entities[0], &prev_level->entities[0], sizeof(Entity));
	}
	level.entity_count = 1;

	// Level Generation
	for (int x=1; x<LEVEL_WIDTH-1; x++) {
		Wall(&level.entities[level.entity_count++], x, 0);
		Wall(&level.entities[level.entity_count++], x, LEVEL_HEIGHT-1);
	}
	for (int y=1; y<LEVEL_HEIGHT-1; y++) {
		Wall(&level.entities[level.entity_count++], 0, y);
		Wall(&level.entities[level.entity_count++], LEVEL_WIDTH-1, y);
	}

	Goblin(&level.entities[level.entity_count++], 2, 2);

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
	for (int y=0; y<LEVEL_HEIGHT; y++) {
		for (int x=0; x<LEVEL_WIDTH; x++) {
			char c = tiles[x][y];
			printf("%c", c ? c : ' ');
		}
		printf("\n");
	}
}

typedef struct {
	int distance[LEVEL_WIDTH][LEVEL_HEIGHT];
	Vector2Int target;
} PathfindingResult;

/**
 * Uses depth-first search to find the shortest distance to target from any point in the level
 * You should run make_lookup(&level) before calling this function to update the reverse tile lookup
 */
PathfindingResult pathfind(Level* level, Vector2Int target) {
	PathfindingResult result = {0};
	result.target = target;
	Vector2Int queue[4096];
	int queue_distance[4096];
	int front = 0;
	int back = 0;
	queue[back++] = target;
	queue_distance[back] = 0;

	while (front < back) {
		Vector2Int p = queue[front++];
		int distance = queue_distance[front];
		if (result.distance[p.x][p.y] != 0) continue;
		// If it's a wall, eat it
		EntityIdList* es = &level->by_tile[p.x][p.y];
		int blocked = 0;
		for (size_t i=0; i<es->count; i++) {
			size_t e_id = es->entity_ids[i];
			Entity* e = &level->entities[e_id];
			if (e->blocking) blocked = 1;
		}
		if (target.x == p.x && target.y == p.y) {
			blocked = 0;
			result.distance[p.x][p.y] = 0;
		}
		if (blocked) {
			result.distance[p.x][p.y] = 255;
		} else {
			result.distance[p.x][p.y] = distance;
			for (int x=p.x-1; x<=p.x+1; x++) {
				for (int y=p.y-1; y<=p.y+1; y++) {
					if (x < 0) continue;
					if (y < 0) continue;
					if (x >= LEVEL_WIDTH) continue;
					if (y >= LEVEL_HEIGHT) continue;
					if (x == p.x && y == p.y) continue;
					if (result.distance[x][y] == 0) {
						queue[back++] = (Vector2Int){x,y};
						queue_distance[back] = distance + 1;
					}
				}
			}
		}
	}
	result.distance[target.x][target.y] = 0;
	return result;
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
			result.entity_ids[result.count++] = i;
		}
	}
	return result;
}

char log_buf[1024];
int deal_damage(Level* level, size_t attacker_id, size_t target_id, int damage) {
	Entity* attacker = &level->entities[attacker_id];
	Entity* target = &level->entities[target_id];
	target->hp -= damage;
	sprintf(log_buf, "%s did %d damage to %s\n", attacker->name, damage, target->name);
	log_msg(level->logger, log_buf);
	if (target->hp <= 0) {
		target->type = NONE;
		sprintf(log_buf, "%s died\n", target->name);
		log_msg(level->logger, log_buf);
	}
	return damage;
}

int entity_attack(Level* level, size_t attacker_id, size_t target_id) {
	Entity* attacker = &level->entities[attacker_id];
	if (attacker->type == NONE) return -1;
	if (++attacker->impetus_to_attack >= attacker->attack_delay) {
		attacker->impetus_to_attack = 0;
		return deal_damage(level, attacker_id, target_id, attacker->damage);
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
	if (entity->type == NONE) return -1;
	if (entity->inverse_speed == 0) return -1;

	PathfindingResult path = pathfind(level, target);
	Vector2Int p = entity->position;
	int current_dist = path.distance[p.x][p.y];
	Vector2Int dir = {0};
	for (int x=p.x-1; x<=p.x+1; x++) {
		for (int y=p.y-1; y<=p.y+1; y++) {
			if (x < 0) continue;
			if (y < 0) continue;
			if (x >= LEVEL_WIDTH) continue;
			if (y >= LEVEL_HEIGHT) continue;
			if (x == p.x && y == p.y) continue;
			if (path.distance[x][y] < current_dist) {
				dir.x = x-p.x;
				dir.y = y-p.y;
				current_dist = path.distance[x][y];
			}
		}
	}
	if (dir.x == 0 && dir.y == 0) return -1;
	Vector2Int desired_position = vec2add(entity->position, dir);

	EntityIdList entities_there = entities_at_location(level, desired_position);
	for (size_t i=0; i<entities_there.count; i++) {
		size_t e_id = entities_there.entity_ids[i];
		Entity* e = &level->entities[e_id];
		if (e->bumpable) return entity_attack(level, entity_id, e_id);
		if (e->blocking) return -1;
	}
	entity->impetus_to_move++;
	if (entity->impetus_to_move >= entity->inverse_speed) {
		entity->impetus_to_move = 0;
		entity->position = desired_position;
		for (size_t i=0; i<entities_there.count; i++) {
			size_t e_id = entities_there.entity_ids[i];
			Entity* e = &level->entities[e_id];
			if (e->type == ITEM) {
				// TODO: log this information
				if (e->hp) entity->hp += e->hp;
				if (e->armor) entity->armor += e->armor;
				if (e->arrows) entity->arrows += e->arrows;
				if (e->gold) entity->gold += e->gold;
				e->type = NONE;
			}
		}
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
		case ATTACK: // TODO:
		case WAIT:
		default:
	}

	for (size_t i=0; i<level->entity_count; i++) {
		Entity* e = &level->entities[i];
		if (e->aggro_entity_id >= 0) {
			Entity* target = &level->entities[e->aggro_entity_id];
			if (target->type != NONE) entity_walk(level, i, target->position);
		}
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
