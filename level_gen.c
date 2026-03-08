#include "engine.c"
#include "log.c"
#include <stdbool.h>

typedef struct {
	int top;
	int bottom;
	int left;
	int right;
} rect_t;

rect_t WHOLE_LEVEL = {0, LEVEL_HEIGHT-1, 0, LEVEL_WIDTH-1};

Entity* new(Level* level) {
	return &level->entities[level->entity_count++];
}

void randomize_position(Level* level, Entity* entity);
void randomize_position_bb(Level* level, Entity* e, rect_t bb);

int buy_in_budget(Level* level, rect_t bounding_box, LevelGridInt* map)
{
	int idx = rand() % 8; // Increment this when you add more cases below:
	int cost = 0;
	Entity* e;
	switch (idx) {
		case 0:
			cost = 2;
			e = Goblin(new(level));
			break;
		case 1:
			cost = 1;
			e = SmallGold(new(level));
			break;
		case 2:
			cost = 3;
			e = MediumGold(new(level));
			break;
		case 3:
			cost = 5;
			e = LargeGold(new(level));
			break;
		case 4:
			cost = 5;
			e = Armor(new(level), 18);
			break;
		case 5:
			cost = 5;
			e = Health(new(level), 1);
			break;
		case 6:
			cost = 1;
			e = SingleArrow(new(level));
			break;
		case 7:
			cost = 2;
			e = Arrows(new(level), 3);
			break;
		default:
			return 1;
	}
	do {
		randomize_position_bb(level, e, bounding_box);
	} while (map->tiles[e->position.x][e->position.y]);
	return cost;
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

void bsp_iter(Level* level, unsigned remaining_depth, rect_t bounding_box, LevelGridInt* map)
{
	if (remaining_depth == 0 || bounding_box.bottom - bounding_box.top < 4 || bounding_box.right - bounding_box.left < 4) {
		// TODO perturb these
		unsigned x_start = bounding_box.left + 1;
		unsigned x_end = bounding_box.right - 1;
		unsigned y_start = bounding_box.top + 1;
		unsigned y_end = bounding_box.bottom - 1;

		int max_budget = 0;

		// Carve out the room
		for (unsigned x = x_start; x <= x_end; x++) {
			for (unsigned y = y_start; y <= y_end; y++) {
				map->tiles[x][y] = 0;
				max_budget++;
			}
		}

		// Place monsters in it
		max_budget *= (level->level_number + 1);
		max_budget = ilog2(max_budget);
		int budget = rand() % max_budget;
		while (budget > 0) {
			budget -= buy_in_budget(level, bounding_box, map);
		}

	} else {
		// Bisect the bounding box
		rect_t bb1 = bounding_box;
		rect_t bb2 = bounding_box;

		unsigned split_direction = rand() % 2;
		if (split_direction) {
			int top = bounding_box.top;
			int bottom = bounding_box.bottom;
			// unsigned halfrange = (bottom - top) / 2;
			// unsigned split_point = rand() % halfrange + halfrange / 2 + top;
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
		bsp_iter(level, remaining_depth - 1, bb1, map);
		bsp_iter(level, remaining_depth - 1, bb2, map);

		// Join the sub-partitions

		Vector2Int p1, p2;
		do {
			p1.y = rand() % (bb1.bottom - bb1.top) + bb1.top;
			p1.x = rand() % (bb1.right - bb1.left) + bb1.left;
		} while (map->tiles[p1.x][p1.y]);

		do {
			p2.y = rand() % (bb2.bottom - bb2.top) + bb2.top;
			p2.x = rand() % (bb2.right - bb2.left) + bb2.left;
		} while (map->tiles[p2.x][p2.y]);

		int x_begin = p1.x > p2.x ? p2.x : p1.x;
		int x_end = p1.x > p2.x ? p1.x : p2.x;

		int y_begin = p1.y > p2.y ? p2.y : p1.y;
		int y_end = p1.y > p2.y ? p1.y : p2.y;

		unsigned join_direction = rand() % 2;
		for (int x = x_begin; x <= x_end; x++) {
			map->tiles[x][y_begin] = 0;
		}
		for (int y = y_begin; y <= y_end; y++) {
			map->tiles[x_end][y] = 0;
		}
	}
}

void bsp_dungeon(Level* level)
{
	// Call bsp_iter on the top level
	LevelGridInt map;
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			map.tiles[x][y] = 1;
		}
	}
	bsp_iter(level, 4, WHOLE_LEVEL, &map);
	// For every filled cell in the output tilemap, place a wall.
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (map.tiles[x][y]) {
				Position(Wall(new(level)), x, y);
			}
		}
	}
}

int in_bb(rect_t bb, Vector2Int p) {
	return p.x <= bb.left || p.x > bb.right || p.y <= bb.top || p.y > bb.bottom;
}

/**
 * Will put an entity somewhere random where there isn't anything else
 */
void randomize_position(Level* level, Entity* entity) {
	randomize_position_bb(level, entity, WHOLE_LEVEL);
}

void randomize_position_bb(Level* level, Entity* e, rect_t bb) {
	int width = bb.right - bb.left;
	int height = bb.bottom - bb.top;
	Vector2Int* p = &e->position;
	EntityIdList here;
	do {
		p->x = bb.left + rand() % width;
		p->y = bb.top  + rand() % height;
		here = entities_at_location(level, *p);
	} while (here.count > 1);
}

void generate_level(Level* level)
{
	bsp_dungeon(level);
	randomize_position(level, Staircase(new(level)));
	randomize_position(level, Map(new(level), level->level_number));
}
