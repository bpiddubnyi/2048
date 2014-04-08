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

	wclear(w);

	getmaxyx(w, height, width);
	if (height < BOARD_HEIGHT || width < BOARD_WIDTH) {
		wmove(w, 0, 0);
		wprintw(w, "Terminal window is too small to display gameboard");
		wrefresh(w);
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

	wrefresh(w);
}

static void print_score(WINDOW *w, uint64_t score)
{
	assert(w);

	wclear(w);

	wattron(w, A_BOLD);
	mvwprintw(w, 0, 0, " Score: %" PRIu64, score);
	wattroff(w, A_BOLD);
	wrefresh(w);
}

static void game_loop(void)
{
	int height, width, mv, ch = 0;
	bool exit = false;
	struct game_2048 game;
	WINDOW *stats_w, *board_w;

	getmaxyx(stdscr, height, width);

	stats_w = newwin(1, width, 0, 0);
	board_w = newwin(height - 1, width, 1, 0);
	
	wrefresh(stats_w);
	wrefresh(board_w);

	refresh();

	game_2048_init(&game);
	while (!exit) {
		print_board(board_w, &game);
		print_score(stats_w, game.score);

		if (game_2048_is_over(&game))
			exit = true;

		ch = getch();
		switch(ch) {
		case KEY_LEFT:
			mv = G2048_MOVE_LEFT;
			break;
		case KEY_RIGHT:
			mv = G2048_MOVE_RIGHT;
			break;
		case KEY_DOWN:
			mv = G2048_MOVE_BOTTOM;
			break;
		case KEY_UP:
			mv = G2048_MOVE_TOP;
			break;
		case 'q':
			exit = true;
			break;
		case 'n':
			if (exit == true) {
				exit = false;
				game_2048_init(&game);
			}
		default:
			mv = -1;
		}

		if (mv > -1)
			game_2048_move(&game, mv);
	}

	delwin(board_w);
	delwin(stats_w);
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

	game_loop();

	endwin();
	return 0;
}
