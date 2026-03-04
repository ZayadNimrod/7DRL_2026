
#include "engine.c"
#include "log.c"
#include <ncurses.h>

typedef struct {
    char character;
    int attributes;
} display_t;

display_t entity_char(Entity* entity)
{

    switch (entity->type) {
    case NONE:
        return (display_t) { .character = ' ', .attributes = A_NORMAL };
    case PLAYER:
        return (display_t) { .character = '@', .attributes = A_BOLD };
    case WALL:
        return (display_t) { .character = '#', .attributes = A_NORMAL };
    default:
        return (display_t) { .character = '?', .attributes = A_BLINK };
    }
}

WINDOW* map_window;
WINDOW* log_window;

void render_map(Level* world)
{
    wclear(map_window);

    for (unsigned i = 0; i < world->entity_count; i++) {
        Entity* e = &world->entities[i];
        display_t d = entity_char(e);
        wattron(map_window, d.attributes);
	Vector2Int position = e->position;
        mvwaddch(map_window, position.x, position.y, d.character);
        wattroff(map_window, d.attributes);
    }

    wrefresh(map_window);
}

void render_log(logger_t* logger)
{
    wclear(log_window);
    if (logger->total_logs>=logger->max_logs){
        for (unsigned i = 0; i < logger->max_logs; i++) {
            wmove(log_window, i, 0);
            unsigned idx = (logger->idx + i) % logger->max_logs;
            char* log = logger->logs[idx];
            waddstr(log_window, log);
        }
    } else{
        for (unsigned i = 0; i < logger->total_logs; i++) {
            wmove(log_window, i, 0);
            char* log = logger->logs[i];
            waddstr(log_window, log);
        }
    }
    wrefresh(log_window);
}

void render(Level* world, logger_t* logger)
{
    render_map(world);
    render_log(logger);
}

int quit()
{
    curs_set(1); // Restore cursor
    endwin();
    return 0;
}

int main()
{

    initscr();
    curs_set(0); // hide cursor
    noecho();
	clear();
	refresh();

    Level level = { 0 };
    level = init_level(0, &level);

    const int MAX_LOGS = LEVEL_HEIGHT / 2;
    const int MAX_LOG_LEN = 120 - LEVEL_WIDTH;

    map_window = newwin(LEVEL_HEIGHT, LEVEL_WIDTH, 0, 0);
    log_window = newwin(MAX_LOGS, MAX_LOG_LEN, LEVEL_HEIGHT - MAX_LOGS, LEVEL_WIDTH);

    logger_t logger = init_logger(MAX_LOGS, MAX_LOG_LEN);

    log_msg(&logger, "This is a log");
    log_msg(&logger, "This is another log");
    log_msg(&logger, "Here is another log that is so long that it should go over multiple lines beep boop bap bop.");

    while (true) {
        render(&level,&logger);

        char input = getch();

        switch (input) {
        case 'q':
            return quit();
		case '?':
			log_msg(&logger, "TODO: Explain controls");
			break;

        default:
            break;
        }
    }
    return 0;
}
