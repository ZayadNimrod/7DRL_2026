
#include "engine.c"
#include <ncurses.h>

typedef struct {
    char character;
    int attributes;
} display_t;

display_t entity_char(Entity* entity)
{

    switch (entity->type) {
    case NONE:
		return (display_t){ .character = ' ', .attributes = A_NORMAL };
    case PLAYER:
        return (display_t){ .character = '@', .attributes = A_BOLD };
    case WALL:
        return (display_t){ .character = '#', .attributes = A_NORMAL };
    default:
        return (display_t){ .character = '?', .attributes = A_BLINK };
        
    }
}

void render(Level* world)
{
    // Clear the screen
    move(0, 0);
    char blankline[LEVEL_WIDTH + 1];
    for (int i = 0; i < LEVEL_WIDTH; i++) {
        blankline[i] = ' ';
    }
    blankline[LEVEL_WIDTH] = 0;

    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        mvaddstr(y, 0,blankline);
    }

    for (unsigned i = 0; i < world->entity_count; i++) {
        Entity* e = &world->entities[i];
		display_t d = entity_char(e);
		attron(d.attributes);
        mvaddch(e->y, e->x, d.character);
		attroff(d.attributes);
    }

    refresh(); /* Print it on to the real screen */
}

int quit()
{
    curs_set(1); // Restore cursor
    endwin();
    return 0;
}

int main()
{

    Level level = { 0 };
    level = init_level(0, &level);

    initscr(); /* Start curses mode 		    */
    curs_set(0); // hide cursor
    noecho();

    while (true) {
        render(&level);

        char input = getch();

        switch (input) {
        case 'q':
            return quit();
        default:
            break;
        }
    }
    return 0;
}
