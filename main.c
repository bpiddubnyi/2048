#include <ncurses.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define BOARD_SIDE 4
#define FIELD_WIDTH 8
#define FIELD_HEIGHT 3
#define BORDERS_NUM (BOARD_SIDE + 1)
#define BOARD_WIDTH ((FIELD_WIDTH * BOARD_SIDE) + BORDERS_NUM)
#define BOARD_HEIGHT ((FIELD_HEIGHT * BOARD_SIDE) + BORDERS_NUM)
#define CELL_PADDING ((FIELD_HEIGHT - 1) / 2)

static uint16_t board[BOARD_SIDE][BOARD_SIDE] = { { 2048 } };

static void print_cell(WINDOW *w, size_t row, size_t col)
{
	char buf[FIELD_WIDTH];
	int len, left, right;

	assert(row < BOARD_SIDE);
	assert(col < BOARD_SIDE);

	len = snprintf(buf, FIELD_WIDTH, "%" PRIu16, board[row][col]);
	left = (FIELD_WIDTH - len) / 2;
	right = FIELD_WIDTH - len -left;

	attron(A_BOLD);
	wprintw(w, "%*s%" PRIu16 "%*s", left, " ", board[row][col], right, " ");
	attroff(A_BOLD);
}

static void print_row(WINDOW *w, size_t row)
{
	assert(w);

	waddch(w, '|');
	for (int col = 0; col < BOARD_SIDE; ++col) {
		print_cell(w, row, col);
		waddch(w, '|');
	}
}

static void print_table_row(WINDOW *w, bool empty)
{
	assert(w);

	waddch(w, empty ? '|' : '+');
	for (size_t cell = 0; cell < BOARD_SIDE; ++cell) {
		for (size_t ch = 0; ch < FIELD_WIDTH; ++ch) {
			waddch(w, empty ? ' ' : '-');
		}
		waddch(w, empty ? '|' : '+');
	}
}

static int print_board(WINDOW *w) {
	int height, width, left, top;
	assert(w);

	getmaxyx(w, height, width);
	if (height < BOARD_HEIGHT || width < BOARD_WIDTH) {
		wprintw(w, "Terminal window is too small to display gameboard");
		return -1;
	}

	top = (height - BOARD_HEIGHT) / 2;
	left = (width - BOARD_WIDTH) / 2;

	wmove(w, top, left);
	print_table_row(w, false);
	for (size_t row = 0; row < BOARD_SIDE; ++row) {
		for (size_t pad = 0; pad < CELL_PADDING; ++pad) {
			wmove(w, ++top, left);
			print_table_row(w, true);
		}

		wmove(w, ++top, left);
		print_row(w, row);
		
		for (size_t pad = 0; pad < CELL_PADDING; ++pad) {
			wmove(w, ++top, left);
			print_table_row(w, true);
		}

		wmove(w, ++top, left);
		print_table_row(w, false);
	}

	refresh();
	return 0;
}

int main() {
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();

	print_board(stdscr);

	getch();
	endwin();

	return 0;
}
