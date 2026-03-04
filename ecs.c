#ifndef ECS_C
#define ECS_C

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
#define LEVEL_WIDTH 40
#define LEVEL_HEIGHT 25



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


#endif
