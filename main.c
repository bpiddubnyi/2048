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

static void print_board(WINDOW *w, struct game_2048 *g) {
	int height, width, left, top;
	assert(w);

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

static void print_score(WINDOW *w, uint64_t score)
{
	assert(w);

	wclear(w);
	wmove(w, 0, 0);

	wattron(w, A_BOLD);
	wprintw(w,"Score: %" PRIu64, score);
	wattroff(w, A_BOLD);

	wnoutrefresh(w);
}

static void print_menu_offer(WINDOW *w)
{
	assert(w);

	wclear(w);
	wmove(w, 0, 0);
	
	wattron(w, A_BOLD);
	wprintw(w, " Press 'm' for menu");
	wattroff(w, A_BOLD);

	wnoutrefresh(w);
}

enum game_menu {
	MENU_QUIT,
	MENU_NEW_GAME,
	MENU_CONTINUE
};

static int show_game_menu(WINDOW *w, bool win, bool show_continue)
{
	int ch;

	wclear(w);
	wmove(w, 0, 0);

	if (win) {
		wattron(w, A_BOLD);
		wprintw(w, " YOU WIN ");
		wattroff(w, A_BOLD);
	} else if (!show_continue) {
		wattron(w, A_BOLD);
		wprintw(w, " GAME OVER ");
		wattroff(w, A_BOLD);
	}
	
	wprintw(w, "Start a new game(n) / exit(q)%s? ", 
		show_continue ? " / continue(c)" : "");
	wrefresh(w);
	
	while (true) {
		ch = getch();
		switch(ch) {
		case 'n':
			return MENU_NEW_GAME;
		case 'q':
			return MENU_QUIT;
		case 'c':
			if (!show_continue)
				break;
			return MENU_CONTINUE;
		}
	}
	return true;
}

enum game_action {
	ACT_UPDATE,
	ACT_RESIZE,
	ACT_SHOW_MENU
};

static int game_action(struct game_2048 *g)
{
	int ch;
	int mv = -1;

	assert(g);

	while(true) {
		ch = getch();
		switch(ch) {
		case KEY_LEFT:
			mv = G2048_MOVE_LEFT;
			goto action;
		case KEY_RIGHT:
			mv = G2048_MOVE_RIGHT;
			goto action;
		case KEY_DOWN:
			mv = G2048_MOVE_BOTTOM;
			goto action;
		case KEY_UP:
			mv = G2048_MOVE_TOP;
			goto action;
		case KEY_RESIZE:
			return ACT_RESIZE;
		case 'm':
			return ACT_SHOW_MENU;
		}
	}

action:
	game_2048_move(g, mv);

	return ACT_UPDATE;
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
}

static void main_loop(void)
{
	bool exit = false;
	int action;
	struct game_2048 game;
	struct window_layout layout = { 0 };

	layout_init(&layout);
	refresh();

	game_2048_init(&game);
	while (!exit) {
		print_score(layout.score_w, game.score);
		print_board(layout.board_w, &game);
		print_menu_offer(layout.menu_w);
		doupdate();

		action = -1;
		if (game.win || game_2048_is_over(&game) 
		    || ((action = game_action(&game)) == ACT_SHOW_MENU)) { 
			switch(show_game_menu(layout.menu_w, game.win, 
					      (action == ACT_SHOW_MENU) ? true
					      : false)) {
			case MENU_QUIT:
				exit = true;
				continue;
			case MENU_NEW_GAME:
				game_2048_init(&game);
				continue;
			case MENU_CONTINUE:
				continue;
			}
		}
		if (action == ACT_RESIZE)
			layout_init(&layout);
	}

	layout_destroy(&layout);
}

static void define_colors(void)
{
	for (int i = 1; i <= 10; ++i) {
		init_pair(i, i, COLOR_BLACK);
	}
}

int main() {
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();

	if (has_colors()) {
		start_color();
		define_colors();
	}

	main_loop();

	endwin();
	return 0;
}
