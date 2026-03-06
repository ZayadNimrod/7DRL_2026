
#include "engine.c"
#include "log.c"
#include <ncurses.h>
#include <stdlib.h>

typedef struct {
	char character;
	int attributes;
} display_t;

display_t entity_char(Entity* entity)
{
	switch (entity->type) {
	case NONE:
		return (display_t) { .character = 0, .attributes = A_NORMAL };
	case PLAYER:
		return (display_t) { .character = '@', .attributes = A_BOLD };
	case WALL:
		return (display_t) { .character = '#', .attributes = A_NORMAL };
	case ENEMY:
		return (display_t) { .character = 'e', .attributes = COLOR_PAIR(1) };
	case STAIRCASE:
		return (display_t) { .character = '>', .attributes = A_BOLD };
	default:
		return (display_t) { .character = '?', .attributes = A_BLINK };
	}
}

WINDOW* map_window;
WINDOW* log_window;
WINDOW* stat_window;

void render_stats(Level* world)
{
	(void)world;
}

void render_map(Level* world)
{
	wclear(map_window);

	for (int i = world->entity_count - 1; i >= 0; i--) {
		Entity* e = &world->entities[i];
		display_t d = entity_char(e);
		if (d.character != 0) {
			wattron(map_window, d.attributes);
			Vector2Int position = e->position;
			mvwaddch(map_window, position.y, position.x, d.character);
			wattroff(map_window, d.attributes);
			wattron(map_window, COLOR_PAIR(0));
		}
	}

	wrefresh(map_window);
}

void render_log(logger_t* logger)
{
	wclear(log_window);
	if (logger->total_logs >= logger->max_logs) {
		for (unsigned i = 0; i < logger->max_logs; i++) {
			wmove(log_window, i, 0);
			unsigned idx = (logger->idx + i) % logger->max_logs;
			char* log = logger->logs[idx];
			waddstr(log_window, log);
		}
	} else {
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
	render_stats(world);
}

int quit()
{
	curs_set(1); // Restore cursor
	endwin();
	return 0;
}

int main()
{
	const int MAX_LOGS = LEVEL_HEIGHT / 2;
	const int MAX_LOG_LEN = 80 - LEVEL_WIDTH;
	logger_t logger = *init_logger(MAX_LOGS, MAX_LOG_LEN);

	// srand(time(0));
	srand(0);

	initscr();
	start_color();
	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	// init_pair(2, COLOR_ORANGE, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_CYAN, COLOR_BLACK);
	init_pair(5, COLOR_BLUE, COLOR_BLACK);
	init_pair(6, COLOR_MAGENTA, COLOR_BLACK);

	curs_set(0); // hide cursor
	noecho();
	clear();
	refresh();

	Level level = { 0 };
	level.logger = &logger;
	level = init_level(0, &level);

	map_window = newwin(LEVEL_HEIGHT, LEVEL_WIDTH, 0, 0);
	log_window = newwin(MAX_LOGS, MAX_LOG_LEN, LEVEL_HEIGHT - MAX_LOGS, LEVEL_WIDTH);
	stat_window = newwin(LEVEL_HEIGHT - MAX_LOGS - 1, MAX_LOG_LEN, 0, LEVEL_WIDTH);

	//log_msg(&logger, "This is a log");
	//log_msg(&logger, "This is another log");
	//log_msg(&logger, "Here is another log that is so long that it should go over multiple lines beep boop bap bop.");

	InputAction last_action;
	while (true) {
		render(&level, &logger);

		char input = getch();

		Vector2Int player_position = level.entities[0].position;

		switch (input) {
			case 'q':
				return quit();
			case '?':
				log_msg(&logger, "Use the numpad or hjklyunm (vim keys) to move");
				continue;
				break;
			case 'h': case '4':
				last_action.type   = WALK;
				last_action.target = vec2add(player_position,from_direction(WEST));
				break;
			case 'j': case '2':
				last_action.type   = WALK;
				last_action.target = vec2add(player_position,from_direction(SOUTH));
				break;
			case 'l': case '6':
				last_action.type   = WALK;
				last_action.target = vec2add(player_position,from_direction(EAST));
				break;
			case 'k': case '8':
				last_action.type   = WALK;
				last_action.target = vec2add(player_position,from_direction(NORTH));
				break;
			case 'y': case '7':
				last_action.type   = WALK;
				last_action.target = vec2add(player_position,from_direction(NORTH_WEST));
				break;
			case 'u': case '9':
				last_action.type   = WALK;
				last_action.target = vec2add(player_position,from_direction(NORTH_EAST));
				break;
			case 'm': case '3':
				last_action.type   = WALK;
				last_action.target = vec2add(player_position,from_direction(SOUTH_EAST));
				break;
			case 'n': case '1':
				last_action.type   = WALK;
				last_action.target = vec2add(player_position,from_direction(SOUTH_WEST));
				break;
			case '.': case '5':
				last_action.type   = WAIT;
				last_action.target = player_position;
				break;
			case '>':
				last_action.type = DESCEND;
				last_action.target = player_position;
				break;
			default:
				continue;
		}

		while (tick_level(&level, last_action)) { }
	}
	return 0;
}
