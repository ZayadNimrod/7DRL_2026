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

void bsp_iter(Level* level, unsigned remaining_depth, rect_t bounding_box, char* tilemap, unsigned width, unsigned height)
{
    if (remaining_depth == 0 || bounding_box.top - bounding_box.bottom < 4 || bounding_box.right - bounding_box.left < 4) {
        // TODO perturb these
        unsigned x_start = bounding_box.left + 1;
        unsigned x_end = bounding_box.right - 1;
        unsigned y_start = bounding_box.bottom + 1;
        unsigned y_end = bounding_box.top - 1;

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
        rect_t bb1 = bounding_box;
        rect_t bb2 = bounding_box;

        unsigned split_direction = rand() % 2;
        if (split_direction) {
            int top = bounding_box.top;
            int bottom = bounding_box.bottom;
            unsigned halfrange = (top - bottom) / 2;
            unsigned split_point = rand() % halfrange + halfrange / 2 + bottom;
            bb1.top = top;
            bb1.bottom = split_point;
            bb2.top = split_point;
            bb2.bottom = bottom;

        } else {
            int right = bounding_box.right;
            int left = bounding_box.left;
            unsigned halfrange = (right - left) / 2;
            unsigned split_point = rand() % halfrange + halfrange / 2 + left;
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
        }while (tilemap[p2.y * width + p2.x]);

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
    bsp_iter(level, 3, (rect_t) { .bottom = 0, .top = height - 1, .left = 0, .right = width - 1 }, tilemap, width, height);

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
}
