
#include "engine.c"
#include "log.c"
#include "colors.h"
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

	Entity* player = &level->entities[0];
	LevelGridInt player_vis = player_vision(level);
	for (int i = level->entity_count - 1; i >= 0; i--) {
		Entity* e = &level->entities[i];
		if (e->type != NONE && player->map == level->level_number) {
			e->explored = 1;
		}
		int attributes = e->attributes;
		wcolor_set(map_window, e->color_pair, NULL);
		if (e->type==NONE || !e->explored) {
			continue;
		}
		else if (player_vis.tiles[e->position.x][e->position.y] == 0) {
			if (!e->is_static) continue;
			wcolor_set(map_window, e->color_pair+128, NULL);
		}
		wattron(map_window, attributes);
		Vector2Int position = e->position;
		mvwaddch(map_window, position.y, position.x, e->avatar);
		wattroff(map_window, attributes);

		wcolor_set(map_window, 0, NULL);
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


Vector2Int char2target(char input, Vector2Int player_position){
	switch (input){
		case 'h': case '4':
			return vec2add(player_position,from_direction(WEST));
		case 'j': case '2':
			return vec2add(player_position,from_direction(SOUTH));
		case 'l': case '6':
			return vec2add(player_position,from_direction(EAST));
		case 'k': case '8':
			return vec2add(player_position,from_direction(NORTH));
		case 'y': case '7':
			return vec2add(player_position,from_direction(NORTH_WEST));
		case 'u': case '9':
			return vec2add(player_position,from_direction(NORTH_EAST));
		case 'm': case '3':
			return vec2add(player_position,from_direction(SOUTH_EAST));
		case 'n': case '1':
			return vec2add(player_position,from_direction(SOUTH_WEST));
		default:
			return player_position;
	}
}

int main()
{
	const int MAX_LOGS = LEVEL_HEIGHT / 2;
	const int MAX_LOG_LEN = 100 - LEVEL_WIDTH;
	logger_t logger = *init_logger(MAX_LOGS, MAX_LOG_LEN);

	// srand(time(0));
	srand(0);

	initscr();
	define_colors();
	color_set(0, NULL);

	curs_set(0); // hide cursor
	noecho();
	clear();
	refresh();

	Level level = { 0 };
	level.logger = &logger;
	level = init_level(1, &level);

	map_window = newwin(LEVEL_HEIGHT, LEVEL_WIDTH, 0, 0);
	log_window = newwin(MAX_LOGS, MAX_LOG_LEN, LEVEL_HEIGHT - MAX_LOGS, LEVEL_WIDTH);
	stat_window = newwin(LEVEL_HEIGHT - MAX_LOGS - 1, MAX_LOG_LEN, 0, LEVEL_WIDTH);

	InputAction last_action;
	while (true) {
		render(&level, &logger);

		char input = getch();

		Vector2Int player_position = level.entities[0].position;

		switch (input) {
			case 'q':
				return quit();
			case '?':
				log_msg(&logger, "Use the numpad or hjklyunm (vim keys) to move. Shoot arrows with f.");
				continue;
				break;
			case 'h': case '4':
			case 'j': case '2':
			case 'l': case '6':
			case 'k': case '8':
			case 'y': case '7':
			case 'u': case '9':
			case 'm': case '3':
			case 'n': case '1':
				last_action.type   = WALK;
				break;
			case '.': case '5':
				last_action.type   = WAIT;
				last_action.target = player_position;
				break;
			case '>':
				last_action.type = DESCEND;
				last_action.target = player_position;
				break;
			case 'f':
				if(level.entities[0].arrows<=0){
					log_msg(&logger, "You have no arrows to fire.");	
					continue;
				}
				log_msg(&logger, "Choose a direction (numpad/vim-keys):");			
				render(&level, &logger);
				input = getch();
				last_action.target = char2target(input, player_position);
				last_action.type = SHOOT;
				if(last_action.target.x==player_position.x && last_action.target.y == player_position.y){
					log_msg(&logger, "Invalid direction.");
					continue;	
				}
				break;
			default:
				continue;
		}

		if (last_action.type == WALK){
			last_action.target = char2target(input, player_position);
		}

		while (tick_level(&level, last_action)) { }
	}
	return 0;
}
