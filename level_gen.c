#include "ecs.c"
#include "log.c"
#include <stdbool.h>

typedef struct {
	int top;
	int bottom;
	int left;
	int right;
} rect_t;

typedef struct {
	int cost;
	bool individual;
	int quantity; // ignore if individual
	void (*function)(void); // pointer to a callback. Has different args depending on if this is an individual or stack item
} cost_t;

#define COSTS_LEN 8
const cost_t cost_table[COSTS_LEN] = {
	{
		.cost = 2,
		.individual = true,
		.function = (void*)Goblin,
	},
	{
		.cost = 1,
		.individual = false,
		.quantity = 3,
		.function = (void*)Gold,
	},
	{
		.cost = 3,
		.individual = false,
		.quantity = 10,
		.function = (void*)Gold,
	},
	{
		.cost = 5,
		.individual = false,
		.quantity = 18,
		.function = (void*)Gold,
	},
	{
		.cost = 5,
		.individual = false,
		.quantity = 2,
		.function = (void*)Armor,
	},
	{
		.cost = 5,
		.individual = false,
		.quantity = 1,
		.function = (void*)Health,
	},
	{
		.cost = 1,
		.individual = false,
		.quantity = 1,
		.function = (void*)Arrows,
	},
	{
		.cost = 2,
		.individual = false,
		.quantity = 3,
		.function = (void*)Arrows,
	}
};

void buy_in_budget(Level* level, int* budget, rect_t bounding_box, char* tilemap, int width)
{
	int idx = rand() % COSTS_LEN;
	cost_t proposed = cost_table[idx];
	if (proposed.cost > *budget)
		return;

	int x;
	int y;

	do {
		x = rand() % (bounding_box.right - bounding_box.left) + bounding_box.left;
		y = rand() % (bounding_box.bottom - bounding_box.top) + bounding_box.top;
	} while (tilemap[y * width + x]);

	if (proposed.individual) {
		*budget -= proposed.cost;
		((void (*)(Entity*, int, int))(proposed.function))(&level->entities[level->entity_count++], x, y);
	} else {
		int count = rand() % (*budget / proposed.cost) + 1;
		*budget -= proposed.cost * count;
		((void (*)(Entity*, int, int, int))(proposed.function))(&level->entities[level->entity_count++], x, y, count * proposed.quantity);
	}
}

int ilog2(int i)
{
	int o = 0;
	while (i) {
		o++;
		i = i >> 1;
	}
	return o;
}

void bsp_iter(Level* level, unsigned remaining_depth, rect_t bounding_box, char* tilemap, unsigned width, unsigned height)
{
	if (remaining_depth == 0 || bounding_box.top - bounding_box.bottom < 4 || bounding_box.right - bounding_box.left < 4) {
		// TODO perturb these
		unsigned x_start = bounding_box.left + 1;
		unsigned x_end = bounding_box.right - 1;
		unsigned y_start = bounding_box.bottom + 1;
		unsigned y_end = bounding_box.top - 1;

		int max_budget = 0;

		// Carve out the room
		for (unsigned x = x_start; x <= x_end; x++) {
			for (unsigned y = y_start; y <= y_end; y++) {
				tilemap[y * width + x] = 0;
				max_budget++;
			}
		}

		// Place monsters in it
		max_budget *= (level->level_number + 1);
		max_budget = ilog2(max_budget);
		int budget = rand() % max_budget;
		while (budget) {
			buy_in_budget(level, &budget, bounding_box, tilemap, width);
		}

	} else {
		// Bisect the bounding box
		rect_t bb1 = bounding_box;
		rect_t bb2 = bounding_box;

		unsigned split_direction = rand() % 2;
		if (split_direction) {
			int top = bounding_box.top;
			int bottom = bounding_box.bottom;
			// unsigned halfrange = (top - bottom) / 2;
			// unsigned split_point = rand() % halfrange + halfrange / 2 + bottom;
			unsigned split_point = (top + bottom) / 2;
			bb1.top = top;
			bb1.bottom = split_point;
			bb2.top = split_point;
			bb2.bottom = bottom;

		} else {
			int right = bounding_box.right;
			int left = bounding_box.left;
			// unsigned halfrange = (right - left) / 2;
			// unsigned split_point = rand() % halfrange + halfrange / 2 + left;
			unsigned split_point = (right + left) / 2;
			bb1.left = left;
			bb1.right = split_point;
			bb2.left = split_point;
			bb2.right = right;
		}

		// Build the sub-partitions
		bsp_iter(level, remaining_depth - 1, bb1, tilemap, width, height);
		bsp_iter(level, remaining_depth - 1, bb2, tilemap, width, height);

		// Join the sub-partitions

		Vector2Int p1, p2;
		do {
			p1.y = rand() % (bb1.top - bb1.bottom) + bb1.bottom;
			p1.x = rand() % (bb1.right - bb1.left) + bb1.left;
		} while (tilemap[p1.y * width + p1.x]);

		do {
			p2.y = rand() % (bb2.top - bb2.bottom) + bb2.bottom;
			p2.x = rand() % (bb2.right - bb2.left) + bb2.left;
		} while (tilemap[p2.y * width + p2.x]);

		int x_begin = p1.x > p2.x ? p2.x : p1.x;
		int x_end = p1.x > p2.x ? p1.x : p2.x;

		int y_begin = p1.y > p2.y ? p2.y : p1.y;
		int y_end = p1.y > p2.y ? p1.y : p2.y;

		unsigned join_direction = rand() % 2;
		for (int x = x_begin; x <= x_end; x++) {
			tilemap[y_begin * width + x] = 0;
		}
		for (int y = y_begin; y <= y_end; y++) {
			tilemap[y * width + x_end] = 0;
		}
	}
}

void bsp_dungeon(Level* level, int width, int height)
{
	// Call bsp_iter on the top level
	char* tilemap = malloc(sizeof(char) * width * height);
	for (int i = 0; i < width * height; i++) {
		tilemap[i] = 1;
	}
	bsp_iter(level, 4, (rect_t) { .bottom = 0, .top = height - 1, .left = 0, .right = width - 1 }, tilemap, width, height);

	// For every filled cell in the output tilemap, place a wall.
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (tilemap[y * width + x]) {
				Wall(&level->entities[level->entity_count++], x, y);
			}
		}
	}
	free(tilemap);
}

void generate_level(Level* level)
{
	bsp_dungeon(level, LEVEL_WIDTH, LEVEL_HEIGHT);

	Vector2Int stair_pos;

	while (1) {
		stair_pos.x = rand() % LEVEL_WIDTH;
		stair_pos.y = rand() % LEVEL_HEIGHT;

		if (!is_walled(level,stair_pos)){
			break;
		}
	}

	Staircase(&level->entities[level->entity_count++],stair_pos.x,stair_pos.y);
}
