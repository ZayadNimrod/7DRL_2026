
#include "engine.c"
#include "log.c"
#include <ncurses.h>
#include <stdlib.h>

WINDOW* map_window;
WINDOW* log_window;
WINDOW* stat_window;

void render_stats(Level* level)
{

	wclear(stat_window);
	Entity* player = &level->entities[0];

	char numbuf[3] = {0};
	wmove(stat_window,1,1);
	// TODO: FIX where number gets shorter display issue
	waddstr(stat_window, "HP:");
	sprintf(numbuf, "%i",player->hp);
	waddstr(stat_window,numbuf);
	wmove(stat_window,2,1);
	waddstr(stat_window, "Armour:");
	sprintf(numbuf, "%i",player->armor);
	waddstr(stat_window,numbuf);
	wmove(stat_window,3,1);
	waddstr(stat_window, "Arrows:");
	sprintf(numbuf, "%i",player->arrows);
	waddstr(stat_window,numbuf);
	wmove(stat_window,4,1);
	waddstr(stat_window, "Gold:");
	sprintf(numbuf, "%i",player->gold);
	waddstr(stat_window,numbuf);

	wrefresh(stat_window);
}

void render_map(Level* level)
{
	wclear(map_window);

	LevelGridInt player_vis = player_vision(level);
	for (int i = level->entity_count - 1; i >= 0; i--) {
		Entity* e = &level->entities[i];

		int attributes = 0;
		if (e->type==NONE || !e->explored) {
			attributes |= A_INVIS;
		}
		else if (player_vis.tiles[e->position.x][e->position.y] == 0) {
			attributes |= A_DIM;
		}
		wattron(map_window, COLOR_PAIR(e->color_pair));
		wattron(map_window, attributes);
		Vector2Int position = e->position;
		mvwaddch(map_window, position.y, position.x, e->avatar);
		wattroff(map_window, COLOR_PAIR(e->color_pair));
		wattroff(map_window, attributes);

		wattron(map_window, COLOR_PAIR(0));
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

void render(Level* level, logger_t* logger)
{
	render_map(level);
	render_log(logger);
	render_stats(level);
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
	const int MAX_LOG_LEN = 100 - LEVEL_WIDTH;
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
	init_pair(7, COLOR_BLACK, COLOR_BLACK);

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
