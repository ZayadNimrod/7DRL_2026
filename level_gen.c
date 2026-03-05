#include "ecs.c"
#include <stdlib.h>

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

typedef struct {
    int top;
    int bottom;
    int left;
    int right;
} rect_t;

void bsp_iter(Level* level, unsigned remaining_depth, rect_t* bounding_box, char* tilemap, unsigned width, unsigned height)
{
    if (remaining_depth == 0 || bounding_box->top - bounding_box->bottom < 4 || bounding_box->right - bounding_box->left < 4) {
        // TODO perturb these
        unsigned x_start = bounding_box->left + 1;
        unsigned x_end = bounding_box->right - 1;
        unsigned y_start = bounding_box->bottom + 1;
        unsigned y_end = bounding_box->top - 1;

        // Carve out the room
        for (unsigned x = x_start; x <= x_end; x++) {
            for (unsigned y = y_start; y <= y_end; y++) {
                tilemap[y * width + x] = 0;
            }
        }

        // Place monsters in it

        // TODO

    } else {
        // Bisect the bounding box
        rect_t bb1 = *bounding_box;
        rect_t bb2 = *bounding_box;

        unsigned split_direction = rand() % 2;
        if (split_direction) {
            int top = bounding_box->top;
            int bottom = bounding_box->bottom;
            unsigned halfrange = (top - bottom) / 2;
            unsigned split_point = rand() % halfrange + halfrange / 2 + bottom;
            bb1.top = top;
            bb1.bottom = split_point;
            bb2.top = split_point;
            bb2.bottom = bottom;

        } else {
            int right = bounding_box->right;
            int left = bounding_box->left;
            unsigned halfrange = (right - left) / 2;
            unsigned split_point = rand() % halfrange + halfrange / 2 + left;
            bb1.left = left;
            bb1.right = split_point;
            bb2.left = split_point;
            bb2.right = right;
        }

        // Build the sub-partitions
        bsp_iter(level, remaining_depth - 1, &bb1, tilemap, width, height);
        bsp_iter(level, remaining_depth - 1, &bb2, tilemap, width, height);

        // Join the sub-partitions
        Vector2Int p1, p2;

        // TODO
    }
}

void bsp_dungeon(Level* level)
{

    int width = LEVEL_WIDTH;
    int height = LEVEL_HEIGHT;
    // Call bsp_iter on the top level
    char tilemap[width * height];
    for (int i = 0; i < width * height; i++) {
        tilemap[i] = 1;
    }
    bsp_iter(level, 4, &(rect_t) { 0, height - 1, 0, width - 1 }, tilemap, width, height);

    // For every filled cell in the output tilemap, place a wall.
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (tilemap[y * width + x]) {
                Wall(&level->entities[level->entity_count++], x, y);
            }
        }
    }
}

void generate_level(Level* level)
{

    // Level Generation
    for (int x = 1; x < LEVEL_WIDTH - 1; x++) {
        add_wall(level, x, 0);
        add_wall(level, x, LEVEL_HEIGHT - 1);
    }
    for (int y = 1; y < LEVEL_HEIGHT - 1; y++) {
        add_wall(level, 0, y);
        add_wall(level, LEVEL_WIDTH - 1, y);
    }

    Entity* goblin = &(level->entities[level->entity_count++]);
    goblin->type = ENEMY;
    goblin->position = (Vector2Int) { 2, 2 };
    goblin->hp = 20;
    goblin->damage = 2;
}
