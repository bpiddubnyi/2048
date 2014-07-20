/*  This file is part of 2048.

    2048 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    2048 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 2048.  If not, see <http://www.gnu.org/licenses/>. */

#include <ncurses.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "2048.h"

#define FIELD_WIDTH  8
#define FIELD_HEIGHT 3
#define BORDERS_NUM (G2048_BOARD_SIDE + 1)
#define BOARD_WIDTH ((FIELD_WIDTH * G2048_BOARD_SIDE) + BORDERS_NUM)
#define BOARD_HEIGHT ((FIELD_HEIGHT * G2048_BOARD_SIDE) + BORDERS_NUM)
#define CELL_PADDING ((FIELD_HEIGHT - 1) / 2)

static void print_cell(WINDOW *w, struct game_2048 *g, size_t row, size_t col)
{
	char buf[FIELD_WIDTH];
	int color, len, left, right;

	assert(g);
	assert(row < G2048_BOARD_SIDE);
	assert(col < G2048_BOARD_SIDE);

	if (g->board[row][col] == 0) {
		wprintw(w, "%*s", FIELD_WIDTH, " ");
		return;
	}

	len = snprintf(buf, FIELD_WIDTH, "%" PRIu16, g->board[row][col]);
	left = (FIELD_WIDTH - len) / 2;
	right = FIELD_WIDTH - len -left;

	color = (int)log2(g->board[row][col] / 2);

	wattron(w, A_BOLD | COLOR_PAIR(color));
	wprintw(w, "%*s%" PRIu16 "%*s", left, " ", g->board[row][col], right, " ");
	wattroff(w, A_BOLD | COLOR_PAIR(color));
}

static void print_board_row(WINDOW *w, struct game_2048 *g, size_t row)
{
	assert(w);

	waddch(w, '|');
	for (int col = 0; col < G2048_BOARD_SIDE; ++col) {
		print_cell(w, g, row, col);
		waddch(w, '|');
	}
}

static void print_table_row(WINDOW *w, bool empty)
{
	assert(w);

	waddch(w, empty ? '|' : '+');
	for (size_t cell = 0; cell < G2048_BOARD_SIDE; ++cell) {
		for (size_t ch = 0; ch < FIELD_WIDTH; ++ch) {
			waddch(w, empty ? ' ' : '-');
		}
		waddch(w, empty ? '|' : '+');
	}
}

static void print_board(WINDOW *w, struct game_2048 *g, bool clear) {
	int height, width, left, top;
	assert(w);

	if (clear)
		wclear(w);

	getmaxyx(w, height, width);
	if (height < BOARD_HEIGHT || width < BOARD_WIDTH) {
		wmove(w, 0, 0);
		wprintw(w, "Terminal window is too small to display gameboard");
		wnoutrefresh(w);
		return;
	}

	top = (height - BOARD_HEIGHT) / 2;
	left = (width - BOARD_WIDTH) / 2;

	wmove(w, top, left);
	print_table_row(w, false);
	for (size_t row = 0; row < G2048_BOARD_SIDE; ++row) {
		for (size_t pad = 0; pad < CELL_PADDING; ++pad) {
			wmove(w, ++top, left);
			print_table_row(w, true);
		}

		wmove(w, ++top, left);
		print_board_row(w, g, row);
		
		for (size_t pad = 0; pad < CELL_PADDING; ++pad) {
			wmove(w, ++top, left);
			print_table_row(w, true);
		}

		wmove(w, ++top, left);
		print_table_row(w, false);
	}

	wnoutrefresh(w);
}

static void print_score(WINDOW *w, uint64_t score, bool clear)
{
	assert(w);

	if (clear)
		wclear(w);
	wmove(w, 0, 0);

	wattron(w, A_BOLD);
	wprintw(w,"Score: ");
	wattroff(w, A_BOLD);
	wprintw(w, "%" PRIu64, score);

	wnoutrefresh(w);
}

enum game_action {
	ACT_RESIZE,
	ACT_SHOW_MENU,
	ACT_QUIT,
	ACT_NEW_GAME,
	ACT_CONTINUE,
	ACT_UPDATE
};

enum game_state {
	STATE_PLAYIN,
	STATE_IN_GAME_MENU,
	STATE_WIN,
	STATE_LOOSE,
};

static void print_menu(WINDOW *w, int state, bool clear)
{
	if (clear)
		wclear(w);
	wmove(w, 0, 0);

	switch(state) {
	case STATE_WIN:
		wattron(w, A_BOLD);
		wprintw(w, "YOU WIN ");
		wattroff(w, A_BOLD);
		break;
	case STATE_LOOSE:
		wattron(w, A_BOLD);
		wprintw(w, "GAME OVER ");
		wattroff(w, A_BOLD);
		break;
	case STATE_PLAYIN:
		wattron(w, A_BOLD);
		wprintw(w, "Press 'm' for menu ");
		wattroff(w, A_BOLD);
		goto end;
	}
	
	wprintw(w, "Start a new game(n) / exit(q)%s? ", 
		state == STATE_IN_GAME_MENU ? " / continue(c)" : "");
end:
	wnoutrefresh(w);
}

static size_t game_action(struct game_2048 *g, int key)
{
	int mv;

	switch(key) {
	case KEY_LEFT:
		mv = G2048_MOVE_LEFT;
		break;
	case KEY_RIGHT:
		mv = G2048_MOVE_RIGHT;
		break;
	case KEY_UP:
		mv = G2048_MOVE_TOP;
		break;
	case KEY_DOWN:
		mv = G2048_MOVE_BOTTOM;
		break;
	default:
		return 0;
	}

	return game_2048_move(g, mv);
}

static int input_handle(int state, struct game_2048 *g)
{
	int ch;

	assert(g);

	while(true) {
		ch = getch();

		switch(ch) {
		case KEY_LEFT:
		case KEY_RIGHT:
		case KEY_DOWN:
		case KEY_UP:
			if (state == STATE_PLAYIN) {
				if (game_action(g, ch))
					return ACT_UPDATE;
				else
					return ACT_CONTINUE;
			}
			break;
		case KEY_RESIZE:
			return ACT_RESIZE;
		case 'm':
			if (state == STATE_PLAYIN)
				return ACT_SHOW_MENU;
			break;
		case 'n':
		case 'q':
			switch(state) {
			case STATE_IN_GAME_MENU:
			case STATE_WIN:
			case STATE_LOOSE:
				if (ch == 'n')
					return ACT_NEW_GAME;
				if (ch == 'q')
					return ACT_QUIT;
			}
			break;
		case 'c':
			if (state == STATE_IN_GAME_MENU)
				return ACT_CONTINUE;
			break;
		}
	}

	return ACT_CONTINUE;
}

struct window_layout {
	WINDOW *score_w;
	WINDOW *board_w;
	WINDOW *menu_w;
};

static void layout_destroy(struct window_layout *layout)
{
	assert(layout);

	if (layout->score_w)
		delwin(layout->score_w);
	if (layout->board_w)
		delwin(layout->board_w);
	if (layout->menu_w)
		delwin(layout->menu_w);
}

static void layout_init(struct window_layout *layout)
{
	int height, width;

	assert(layout);

	layout_destroy(layout);

	getmaxyx(stdscr, height, width);

	layout->score_w = newwin(1, width, 0, 0);
	layout->board_w = newwin(height - 2, width, 1, 0);
	layout->menu_w = newwin(1, width, height - 1, 0);
	refresh();
}

static void main_loop(void)
{
	bool exit = false, clear = true;
	int state = STATE_PLAYIN;
	int action = ACT_RESIZE;
	struct game_2048 game;
	struct window_layout layout = {};

	layout_init(&layout);
	game_2048_init(&game);

	while (!exit) {
		if (game.win) {
			state = STATE_WIN;
		} else if (game_2048_is_over(&game)) {
			state = STATE_LOOSE;
		}

		print_score(layout.score_w, game.score, clear);
		print_board(layout.board_w, &game, clear);
		
menu:
		print_menu(layout.menu_w, state, clear);
		clear = false;
		doupdate();

		switch ((action = input_handle(state, &game))) {
			case ACT_UPDATE:
				continue;
			case ACT_QUIT:
				exit = true;
				continue;
			case ACT_RESIZE:
				layout_init(&layout);
				clear = true;
				continue;
			case ACT_NEW_GAME:
				game_2048_init(&game);
				state = STATE_PLAYIN;
				clear = true;
				continue;
			case ACT_SHOW_MENU:
				state = STATE_IN_GAME_MENU;
				goto menu;
			case ACT_CONTINUE:
				if (state == STATE_IN_GAME_MENU)
					clear = true;
				state = STATE_PLAYIN;
				goto menu;
		}
	}

	layout_destroy(&layout);
}

static void define_colors(void)
{
	for (int i = 1; i <= 10; ++i) {
		init_pair(i, i, -1);
	}
}

int main() {
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();

	if (has_colors()) {
		start_color();
		use_default_colors();
		define_colors();
	}

	main_loop();

	endwin();
	return 0;
}
