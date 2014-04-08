#define _BSD_SOURCE

#include <ncurses.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BOARD_SIDE 4
#define FIELD_WIDTH 8
#define FIELD_HEIGHT 3
#define BORDERS_NUM (BOARD_SIDE + 1)
#define BOARD_WIDTH ((FIELD_WIDTH * BOARD_SIDE) + BORDERS_NUM)
#define BOARD_HEIGHT ((FIELD_HEIGHT * BOARD_SIDE) + BORDERS_NUM)
#define CELL_PADDING ((FIELD_HEIGHT - 1) / 2)

struct game {
	uint64_t score;
	uint16_t board[BOARD_SIDE][BOARD_SIDE];
};

void game_add_random_cell(struct game *g)
{
	assert(g);
	size_t x, y;

	srandom(time(NULL));
	while(true) {
		x = random() % BOARD_SIDE;
		y = random() % BOARD_SIDE;

		if (g->board[x][y] == 0) {
			g->board[x][y] = (x + y) % 2 ? 2 : 4;
			break;
		}
	}
}

void game_init(struct game *g)
{
	assert(g);

	memset(g, 0, sizeof(*g));
	game_add_random_cell(g);
	game_add_random_cell(g);
}

static void print_cell(WINDOW *w, struct game *g, size_t row, size_t col)
{
	char buf[FIELD_WIDTH];
	int len, left, right;

	assert(g);
	assert(row < BOARD_SIDE);
	assert(col < BOARD_SIDE);

	if (g->board[col][row] == 0) {
		wprintw(w, "%*s", FIELD_WIDTH, " ");
		return;
	}

	len = snprintf(buf, FIELD_WIDTH, "%" PRIu16, g->board[col][row]);
	left = (FIELD_WIDTH - len) / 2;
	right = FIELD_WIDTH - len -left;

	attron(A_BOLD);
	wprintw(w, "%*s%" PRIu16 "%*s", left, " ", g->board[col][row], right, " ");
	attroff(A_BOLD);
}

static void print_board_row(WINDOW *w, struct game *g, size_t row)
{
	assert(w);

	waddch(w, '|');
	for (int col = 0; col < BOARD_SIDE; ++col) {
		print_cell(w, g, row, col);
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

static void print_board(WINDOW *w, struct game *g) {
	int height, width, left, top;
	assert(w);

	wclear(w);
	wmove(w, 0, 0);

	getmaxyx(w, height, width);
	if (height < BOARD_HEIGHT || width < BOARD_WIDTH) {
		wprintw(w, "Terminal window is too small to display gameboard");
		wrefresh(w);
		return;
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

	attron(A_BOLD);
	mvwprintw(w, 0, 0, " Score: %" PRIu64, score);
	attroff(A_BOLD);
	wrefresh(w);
}

static void print_key(WINDOW *w, int key)
{
	assert(w);

	attron(A_BOLD);
	mvwprintw(w, 0, 0, " Key pressed: %d", key);
	attroff(A_BOLD);
	wrefresh(w);
}
static void game_loop(void)
{
	int height, width, ch = 0;
	bool exit = false;
	struct game game_state;
	WINDOW *stats_w, *board_w, *key_w;

	getmaxyx(stdscr, height, width);

	stats_w = newwin(1, width / 2, 0, 0);
	key_w = newwin(1, width - (width / 2), 0, width / 2);
	board_w = newwin(height - 1, width, 1, 0);
	
	wrefresh(stats_w);
	wrefresh(key_w);
	wrefresh(board_w);

	refresh();

	game_init(&game_state);
	while (!exit) {
		print_board(board_w, &game_state);
		print_score(stats_w, game_state.score);
		print_key(key_w, ch);

		ch = getch();
		if (ch == 'q')
			break;
	}

	delwin(board_w);
	delwin(stats_w);
}

int main() {
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();

	game_loop();

	endwin();
	return 0;
}
