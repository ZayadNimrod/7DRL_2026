#include "ecs.c"

unsigned add_entity(Level* level, Entity entity)
{
	unsigned ec = level->entity_count++;
	if (ec >= MAX_ENTITIES) {
		// UH OH!!
		printf("We hit the entity limit!\n");
	}
	level->entities[ec] = entity;
	return ec;
}

unsigned add_wall(Level* level, int x, int y)
{
	Entity wall = { 0 };
	wall.type = WALL;
	wall.position = (Vector2Int) { x, y };
	return add_entity(level, wall);
}

void generate_level(Level* level)
{

	// Level Generation
	for (int x = 1; x < LEVEL_WIDTH - 1; x++) {
		Wall(&level.entities[level.entity_count++], x, 0);
		Wall(&level.entities[level.entity_count++], x, LEVEL_HEIGHT - 1);
	}
	for (int y = 1; y < LEVEL_HEIGHT - 1; y++) {
		Wall(&level.entities[level.entity_count++], 0, y);
		Wall(&level.entities[level.entity_count++], LEVEL_WIDTH - 1, y);
	}

	Goblin(&level.entities[level.entity_count++], 2, 2);

	// Spawn some items
	Health(&level.entities[level.entity_count++], 5, 5, 10);
	Gold(&level.entities[level.entity_count++], 8, 10, 10);
	Armor(&level.entities[level.entity_count++], 15, 15, 10);
	Arrows(&level.entities[level.entity_count++], 20, 20, 10);
}
