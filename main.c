
#include <ncurses.h>
#include "engine.c"




char entity_char(Entity* entity) {
	switch (entity->type) {
		case NONE: return ' ';
		case PLAYER: return '@';
		case WALL: return '#';
		default: return ' ';
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
        mvaddch(e->y,e->x, entity_char(e));
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
