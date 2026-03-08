#ifndef GAME_COLORS
#define GAME_COLORS
#include <stdio.h>
#include <ncurses.h>

enum GameColors {
	GC_default,
	GC_player,
	GC_wall,
	GC_goblin,
	GC_bat,
	GC_mushroom,
	GC_snake,
	GC_badger,
	GC_gold,
	GC_armor,
	GC_staircase,
	GC_health,
	GC_arrows,
};

#define BG_COLOR COLOR_BLACK

void pair(short id, short fg, short bg, short dim_fg, short dim_bg) {
	init_pair(id, fg, bg);
	init_pair(id+128, dim_fg, dim_bg);
}

void pair_on_black(short id, short fg, short dim_fg) {
	pair(id, fg, BG_COLOR, dim_fg, BG_COLOR);
}

void define_colors() {
	// Look at the pretty colors: https://upload.wikimedia.org/wikipedia/commons/1/15/Xterm_256color_chart.svg
	start_color();
	pair_on_black(GC_default, 255, 240);
	pair(GC_wall, 52, 215, 215, BG_COLOR);
	pair_on_black(GC_goblin, 199, 53);
	pair_on_black(GC_player, 30, 17);
	pair(GC_gold, BG_COLOR, 220, 58, BG_COLOR);
	pair(GC_staircase, 233, 252, 235, BG_COLOR);
	pair(GC_arrows, 223, BG_COLOR, 130, BG_COLOR);
	pair(GC_health, 196, 255, 124, BG_COLOR); // to piss off the Red Cross (fuck you doctors! I'll use a red cross if I want to)
}

#endif
