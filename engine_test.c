#include <stdio.h>
#include "engine.c"

int main() {
	Level level = {0};
	level = init_level(0, &level);
	printf("%d\n", level.entities[0].type);
	while (1) {
		print_level(&level);
		char input = getchar();
		enum Direction d = STILL;
		switch (input) {
			case 'h': d = WEST; break;
			case 'j': d = SOUTH; break;
			case 'k': d = NORTH; break;
			case 'l': d = EAST; break;
			case 'y': d = NORTH_WEST; break;
			case 'u': d = NORTH_EAST; break;
			case 'n': d = SOUTH_WEST; break;
			case 'm': d = SOUTH_EAST; break;
			default:
		}
		Vector2Int target = vec2add(level.entities[0].position, from_direction(d));
		InputAction action = { WALK, target };
		while (tick_level(&level, action)) { }
	}
	return 0;
}
