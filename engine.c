#include <stdio.h>
#include <string.h>


enum EntityType {
	NONE,
	PLAYER,
	WALL,
	GOBLIN,
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
	int inverse_speed;
	int impetus_to_move; // When this hits inverse_speed, the player can move one tile.
} Entity; 

#define MAX_ENTITIES 4096
#define LEVEL_WIDTH 60
#define LEVEL_HEIGHT 30

typedef struct {
	Entity entities[MAX_ENTITIES];
	unsigned entity_count;
	int level_number;
} Level;
// Entity #0 is always the player

Entity init_player() {
	Entity player = {0};
	player.type = PLAYER;
	player.position = (Vector2Int){ LEVEL_WIDTH/2, LEVEL_HEIGHT/2 };
	player.inverse_speed = 10;
	return player;
};

unsigned add_entity(Level* level, Entity entity) {
	unsigned ec = level->entity_count++;
	if (ec >= MAX_ENTITIES) {
		// UH OH!!
		printf("We hit the entity limit!\n");
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

	return level;
};

void print_level(Level* level) {
	char tiles[LEVEL_WIDTH][LEVEL_HEIGHT] = {0};
	for (unsigned i=0; i<level->entity_count; i++) {
		Entity* e = &level->entities[i];
		tiles[e->position.x][e->position.y] = '.'; // TODO: maybe fix this
	}
	for (int y=0; y<LEVEL_HEIGHT; y++) {
		for (int x=0; x<LEVEL_WIDTH; x++) {
			char c = tiles[x][y];
			printf("%c", c ? c : ' ');
		}
		printf("\n");
	}
}

enum ActionType {
	WAIT, WALK, ATTACK
};

typedef struct {
	enum ActionType type;
	enum Direction direction;
} InputAction;

int entity_walk(Level* level, size_t entity_id, enum Direction direction) {
	Entity* entity = &level->entities[entity_id];
	if (direction != STILL && ++entity->impetus_to_move >= entity->inverse_speed) {
		entity->impetus_to_move = 0;
		entity->position = vec2add(entity->position, from_direction(direction));
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
	enum Direction direction = input.direction;
	switch (input.type) {
		case WALK:
			if (direction >= NORTH && direction <= NORTH_WEST) {
				int has_moved = entity_walk(level, 0 /* player is entity 0 */, direction);
				// Stop ticking the simulation if we've already moved:
				if (!has_moved) get_more_input = 1;
			}
			break;
		default:
	}
	return get_more_input;
}
