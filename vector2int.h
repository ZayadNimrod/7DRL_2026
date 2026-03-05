#ifndef VECTOR2INT
#define VECTOR2INT

typedef struct {
	int x;
	int y;
} Vector2Int;

Vector2Int vec2add(Vector2Int a, Vector2Int b) { Vector2Int result = { a.x + b.x, a.y + b.y }; return result; }

#endif
