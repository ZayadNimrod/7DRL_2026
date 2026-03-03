#include <stdio.h>
#include "engine.c"

int main() {
	Level level = {0};
	level = init_level(0, &level);
	printf("%d\n", level.entities[0].type);
	print_level(&level);
	return 0;
}
