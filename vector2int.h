#ifndef VECTOR2INT
#define VECTOR2INT

typedef struct {
	int x;
	int y;
} Vector2Int;

Vector2Int vec2add(Vector2Int a, Vector2Int b) { Vector2Int result = { a.x + b.x, a.y + b.y }; return result; }
Vector2Int vec2sub(Vector2Int a, Vector2Int b) { Vector2Int result = { a.x - b.x, a.y - b.y }; return result; }

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

#endif
