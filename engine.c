#include <stdio.h>
#include <string.h>


enum EntityType {
	NONE,
	PLAYER,
	WALL,
	GOBLIN,
	TORCH,
};

typedef struct {
	enum EntityType type;
	int x;
	int y;
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
	Entity player;
	player.type = PLAYER;
	player.x = LEVEL_WIDTH/2;
	player.y = LEVEL_HEIGHT/2;
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
	Entity wall;
	wall.type = WALL;
	wall.x = x;
	wall.y = y;
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

char entity_char(Entity* entity) {
	switch (entity->type) {
		case NONE: return ' ';
		case PLAYER: return '@';
		case WALL: return '#';
		default: return ' ';
	}
}

void print_level(Level* level) {
	char tiles[LEVEL_WIDTH][LEVEL_HEIGHT] = {0};
	for (unsigned i=0; i<level->entity_count; i++) {
		Entity* e = &level->entities[i];
		tiles[e->x][e->y] = entity_char(e);
	}
	for (int y=0; y<LEVEL_HEIGHT; y++) {
		for (int x=0; x<LEVEL_WIDTH; x++) {
			char c = tiles[x][y];
			printf("%c", c ? c : ' ');
		}
		printf("\n");
	}
}
