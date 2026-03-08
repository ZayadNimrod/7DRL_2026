#ifndef ENGINE
#define ENGINE

#include "entities.c"
#include "log.c"
#include "vector2int.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define MAX_ENTITIES 4096
#define LEVEL_WIDTH 60
#define LEVEL_HEIGHT 30

int is_in_bounds(Vector2Int position) {
	if (position.x < 0 || position.x >= LEVEL_WIDTH) return 0;
	if (position.y < 0 || position.y >= LEVEL_HEIGHT) return 0;
	return 1;
}

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
// Entity #0 is always the player

typedef struct {
	int tiles[LEVEL_WIDTH][LEVEL_HEIGHT];
} LevelGridInt;

char log_buf[1024];

Entity init_player()
{
	Entity player = { 0 };
	player.type = PLAYER;
	player.position = (Vector2Int) { LEVEL_WIDTH / 2, LEVEL_HEIGHT / 2 };
	player.inverse_speed = 10;
	player.attack_delay = 10;
	player.hp = 10;
	player.damage = 5;
	return player;
};

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


void make_lookup(Level* level) {
	for (int x=0; x<LEVEL_WIDTH; x++) {
		for (int y=0; y<LEVEL_HEIGHT; y++) {
			level->by_tile[x][y] = entities_at_location(level, (Vector2Int){x,y});
		}
	}
}

#include "level_gen.c"



#define WALL_LARGE_NUMBER 100000

typedef struct {
	int distance[LEVEL_WIDTH][LEVEL_HEIGHT];
	Vector2Int target;
} PathfindingResult;

PathfindingResult pathfind(Level* level, Vector2Int target) {
	PathfindingResult result = {0};
	result.target = target;
	for (int x=0; x<LEVEL_WIDTH; x++) {
		for (int y=0; y<LEVEL_HEIGHT; y++) {
			result.distance[x][y] = -1;
			EntityIdList entities_there = level->by_tile[x][y];
			for (size_t i=0; i<entities_there.count; i++) {
				size_t e_id = entities_there.entity_ids[i];
				Entity* e = &level->entities[e_id];
				if (e->blocking) result.distance[x][y] = WALL_LARGE_NUMBER;
			}
		}
	}
	result.distance[target.x][target.y] = 0;
	int changed = 0;
	do {
		changed = 0;
		for (int x=0; x<LEVEL_WIDTH; x++) {
			for (int y=0; y<LEVEL_HEIGHT; y++) {
				if (result.distance[x][y] == -1) {
					for (int xo=-1; xo<=1; xo++) {
						for (int yo=-1; yo<=1; yo++) {
							if (is_in_bounds((Vector2Int){x+xo, y+yo})) continue;
							int d = result.distance[x+xo][y+yo];
							if (d != -1 && d != WALL_LARGE_NUMBER) {
								int new_distance = d+1;
								if (result.distance[x][y] == -1 || result.distance[x][y] > new_distance) {
									result.distance[x][y] = new_distance;
									changed = 1;
								}
							}
						}
					}
				}
			}
		}
	} while (changed);
	return result;
}


typedef struct {
	int tiles[LEVEL_WIDTH][LEVEL_HEIGHT];
} LevelGridInt;


LevelGridInt player_vision(Level* level) {
	LevelGridInt result = {0};
	PathfindingResult player_pf = pathfind(level, level->entities[0].position);
	for (int y = 0; y<LEVEL_HEIGHT; y++) {
		for (int x = 0; x<LEVEL_WIDTH; x++) {
			if (player_pf.distance[x][y] <= level->entities[0].vision) {
				for (int yo=-1; yo<=1; yo++) {
					for (int xo=-1; xo<=1; xo++) {
						if (is_in_bounds((Vector2Int){x+xo, y+yo})) continue;
						result.tiles[x+xo][y+yo] = 1;
					}
				}
			}
		}
	}
	return result;
}

void update_player_vision(Level* level) {
	LevelGridInt vision = player_vision(level);
	for (int y = 0; y<LEVEL_HEIGHT; y++) {
		for (int x = 0; x<LEVEL_WIDTH; x++) {
			if (!vision.tiles[x][y]) continue;
			EntityIdList here = entities_at_location(level,(Vector2Int){x, y});
			for (size_t i = 0; i < here.count; i++) {
				Entity* entity_here = &level->entities[here.entity_ids[i]];
				entity_here->explored = 1;
			}
		}
	}
}


Level init_level(
	int level_number,
	Level* prev_level)
{
	Level level = { 0 };
	level.logger = prev_level->logger;
	level.level_number = level_number;
	if (prev_level->entities[0].type != PLAYER) {
		// Create a new player
		Player(&level.entities[0]);
	} else {
		// Copy the player from the previous level
		memcpy(&level.entities[0], &prev_level->entities[0], sizeof(Entity));
	}
	level.entity_count = 1;

	generate_level(&level);

	// randomise player position
	randomize_position(&level, &level.entities[0]);

	make_lookup(&level);
	update_player_vision(&level);
	return level;
};

void print_level(Level* level)
{
	char tiles[LEVEL_WIDTH][LEVEL_HEIGHT] = { 0 };
	for (unsigned i = 0; i < level->entity_count; i++) {
		Entity* e = &level->entities[i];
		Vector2Int p = e->position;
		if (p.x < 0 || p.y < 0 || p.x >= LEVEL_WIDTH || p.y >= LEVEL_HEIGHT) {
		} else {
			tiles[p.x][p.y] = '.'; // TODO: maybe fix this
		}
	}
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			char c = tiles[x][y];
			printf("%c", c ? c : ' ');
		}
		printf("\n");
	}
}

enum ActionType {
	WAIT,
	WALK,
	ATTACK,
	DESCEND,
	SHOOT
};

typedef struct {
	enum ActionType type;
	Vector2Int target;
} InputAction;

int deal_damage(Level* level, size_t attacker_id, size_t target_id, int damage, bool visible)
{
	Entity* attacker = &level->entities[attacker_id];
	Entity* target = &level->entities[target_id];
	target->hp -= damage;
	if(visible) {
		sprintf(log_buf, "%s did %d damage to %s\n", attacker->name, damage, target->name);
		log_msg(level->logger, log_buf);
	}
	if (target->hp <= 0) {
		target->type = NONE;
		if(visible) {
			sprintf(log_buf, "%s died\n", target->name);
			log_msg(level->logger, log_buf);
		}
		make_lookup(level);
	}
	return damage;
}

int entity_attack(Level* level, size_t attacker_id, size_t target_id, bool visible)
{
	Entity* attacker = &level->entities[attacker_id];
	if (attacker->type == NONE)
		return -1;
	if (++attacker->impetus_to_attack >= attacker->attack_delay) {
		attacker->impetus_to_attack = 0;
		return deal_damage(level, attacker_id, target_id, attacker->damage, visible);
	}
	return 0;
}

/**
 * Tries to walk.
 * Returns 1 if the entity walks successfully
 * Returns 0 if the entity will walk when their impetus is enough
 * Returns -1 if the entity is blocked
 */
int entity_walk(Level* level, size_t entity_id, Vector2Int target)
{
	Entity* entity = &level->entities[entity_id];
	if (entity->type == NONE) return -1;
	if (entity->inverse_speed == 0) return -1;

	Vector2Int dir = { target.x - entity->position.x, target.y - entity->position.y };
	if (dir.x < -1 || dir.x > 1 || dir.y < -1 || dir.y > 1) {
		PathfindingResult path = pathfind(level, target);
		Vector2Int p = entity->position;
		int current_dist = path.distance[p.x][p.y];

		// If you can't see it, you can't walk to it, I guess.
		if (current_dist > entity->vision) return -1;

		for (int x = p.x - 1; x <= p.x + 1; x++) {
			for (int y = p.y - 1; y <= p.y + 1; y++) {
				if (!is_in_bounds((Vector2Int){x, y})) continue;
				if (x == p.x && y == p.y) continue;
				if (path.distance[x][y] < current_dist) {
					dir.x = x - p.x;
					dir.y = y - p.y;
					current_dist = path.distance[x][y];
				}
			}
		}
	}
	if (dir.x == 0 && dir.y == 0) return -1;
	Vector2Int desired_position = vec2add(entity->position, dir);

	EntityIdList entities_there = entities_at_location(level, desired_position);
	for (size_t i = 0; i < entities_there.count; i++) {
		size_t e_id = entities_there.entity_ids[i];
		Entity* e = &level->entities[e_id];
		if (e->bumpable) {
			LevelGridInt player_vis = player_vision(level);
			return entity_attack(level, entity_id, e_id,player_vis.tiles[desired_position.x][desired_position.y]);
		};
		if (e->blocking) return -1;
	}
	entity->impetus_to_move++;
	if (entity->impetus_to_move >= entity->inverse_speed) {
		entity->impetus_to_move = 0;
		entity->position = desired_position;
		for (size_t i = 0; i < entities_there.count; i++) {
			size_t e_id = entities_there.entity_ids[i];
			Entity* e = &level->entities[e_id];
			if (e->type == ITEM) {
				sprintf(log_buf, "Picked up %s", e->name);
				log_msg(level->logger, log_buf);
				if (e->hp) entity->hp += e->hp;
				if (e->armor) entity->armor += e->armor;
				if (e->arrows) entity->arrows += e->arrows;
				if (e->gold) entity->gold += e->gold;
				if (e->map) entity->map = e->map;
				e->type = NONE;
			}
			if (e->type == STAIRCASE && entity_id == 0) {
				sprintf(log_buf, "Press > to go deeper... if you dare >:)");
				log_msg(level->logger, log_buf);
			}
		}
		make_lookup(level);
		return 1;
	}
	return 0;
}

/**
 * Runs the simulation for one game tick.
 * If this returns zero, you should call this again before getting another input
 * from the player.
 */
int tick_level(Level* level, InputAction input)
{
	make_lookup(level);
	int get_more_input = 0;
	switch (input.type) {
		case WALK:
			int has_moved = entity_walk(level, 0 /* player is entity 0 */, input.target);
			// Simluate more ticks unless we've finished moving:
			if (!has_moved) get_more_input = 1;
			break;
		case ATTACK: // TODO:
			break;
		case WAIT:
			break;
		case DESCEND:
			EntityIdList here = entities_at_location(level,level->entities[0].position);
			int staircase_found = 0;
			for (size_t i = 0; i < here.count; i++) {
				Entity* entity_here = &level->entities[here.entity_ids[i]];
				if (entity_here->type == STAIRCASE) staircase_found = 1;
			}
			if (staircase_found) {
				Level new_level = init_level(level->level_number+1, level);
				*level = new_level;
				log_msg(level->logger,"You descend the staircase..");				
			} else {
				log_msg(level->logger,"There is no staircase here.");
			}
			break;
		case SHOOT:
			level->entities[0].arrows--;
			Vector2Int offset = vec2sub(input.target,level->entities[0].position);
			Vector2Int target_position = level->entities[0].position;
			LevelGridInt player_vis = player_vision(level);
			const int SHOOT_RANGE = 10;
			const int ARROW_DAMAGE = 3;
			for(int i =0 ;i< SHOOT_RANGE;i++){
				target_position = vec2add(target_position,offset);
				EntityIdList entities_here = entities_at_location(level,target_position);
				for (size_t j = 0; j < entities_here.count; j++) {
					int entity_id = entities_here.entity_ids[j];
					Entity* entity_here = &level->entities[entity_id];
					if (entity_here->type == WALL) {
						if (player_vis.tiles[target_position.x][target_position.y]){
							log_msg(level->logger,"The arrow bounces off the wall.");
						}else{
							log_msg(level->logger,"You hear the arrow bounce off a wall.");
						}
						goto shootingDone;
					}
					else if (entity_here->type==ENEMY){
						deal_damage(level, 0, entity_id, ARROW_DAMAGE, player_vis.tiles[target_position.x][target_position.y]);
						if (!player_vis.tiles[target_position.x][target_position.y]){
							log_msg(level->logger,"The arrow hits something!");
						}
						goto shootingDone;
					}

				}
			}

			if (player_vis.tiles[target_position.x][target_position.y])
				log_msg(level->logger,"The arrow falls to the ground.");
			shootingDone:		
			
			break;
		default:
			log_msg(level->logger,"Illegal action");
			break;
	}

	for (size_t i = 0; i < level->entity_count; i++) {
		Entity* e = &level->entities[i];
		if (e->aggro_entity_id >= 0) {
			Entity* target = &level->entities[e->aggro_entity_id];
			if (target->type != NONE)
				entity_walk(level, i, target->position);
		}
	}

	if (!get_more_input) {
		update_player_vision(level);
	}

	return get_more_input;
}


#endif
