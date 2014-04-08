#define _BSD_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "2048.h"

/* {{{1 Collapse functions */

/* {{{2 Row collapse */

static void game_2048_collapse_row_left(struct game_2048 *g, size_t row)
{
	ssize_t first_zero = -1;
	for (size_t col = 0; col < G2048_BOARD_SIDE; ++col) {
		if (g->board[row][col] == 0) {
			if (first_zero == -1)
				first_zero = col;
			continue;
		}

		if (first_zero > -1) {
			g->board[row][first_zero++] = g->board[row][col];
			g->board[row][col] = 0;
		}
	}
}

static void game_2048_collapse_row_right(struct game_2048 *g, size_t row)
{
	ssize_t first_zero = -1;
	for (ssize_t col = G2048_BOARD_SIDE - 1; col >= 0; --col) {
		if (g->board[row][col] == 0) {
			if (first_zero == -1)
				first_zero = col;
			continue;
		}

		if (first_zero > -1) {
			g->board[row][first_zero--] = g->board[row][col];
			g->board[row][col] = 0;
		}
	}
}

/* {{{2 Column collapse */

static void game_2048_collapse_col_top(struct game_2048 *g, size_t col)
{
	ssize_t first_zero = -1;
	for (ssize_t row = 0; row < G2048_BOARD_SIDE; ++row) {
		if (g->board[row][col] == 0) {
			if (first_zero == -1)
				first_zero = row;
			continue;
		}

		if (first_zero > -1) {
			g->board[first_zero++][col] = g->board[row][col];
			g->board[row][col] = 0;
		}
	}
}

static void game_2048_collapse_col_bottom(struct game_2048 *g, size_t col)
{
	ssize_t first_zero = -1;
	for (ssize_t row = G2048_BOARD_SIDE - 1; row >= 0; --row) {
		if (g->board[row][col] == 0) {
			if (first_zero == -1)
				first_zero = row;
			continue;
		}

		if (first_zero > -1) {
			g->board[first_zero--][col] = g->board[row][col];
			g->board[row][col] = 0;
		}
	}
}

/* {{{1 Merge functions */

/* {{{2 Row merge */

static void game_2048_merge_row_left(struct game_2048 *g, size_t row) {
	game_2048_collapse_row_left(g, row);
	for (size_t col = 0; col < (G2048_BOARD_SIDE - 1); ++col) {
		/* The board is already collapsed, so there's no cells after
		 * the zero */
		if (g->board[row][col] == 0)
			break;
		if (g->board[row][col] == g->board[row][col + 1]) {
			g->board[row][col] *= 2;
			g->board[row][col + 1] = 0;
			g->score += g->board[row][col];
			game_2048_collapse_row_left(g, row);
		}
	}
}

static void game_2048_merge_row_right(struct game_2048 *g, size_t row) {
	game_2048_collapse_row_right(g, row);
	for (size_t col = (G2048_BOARD_SIDE - 1); col > 0; --col) {
		/* The board is already collapsed, so there's no cells after
		 * the zero */
		if (g->board[row][col] == 0)
			break;
		if (g->board[row][col] == g->board[row][col - 1]) {
			g->board[row][col] *= 2;
			g->board[row][col - 1] = 0;
			g->score += g->board[row][col];
			game_2048_collapse_row_right(g, row);
		}
	}
}

/* {{{2 Column merge */

static void game_2048_merge_col_top(struct game_2048 *g, size_t col) {
	game_2048_collapse_col_top(g, col);
	for (size_t row = 0; row < (G2048_BOARD_SIDE - 1); ++row) {
		/* The board is already collapsed, so there's no cells after
		 * the zero */
		if (g->board[row][col] == 0)
			break;
		if (g->board[row][col] == g->board[row + 1][col]) {
			g->board[row][col] *= 2;
			g->board[row + 1][col] = 0;
			g->score += g->board[row][col];
			game_2048_collapse_col_top(g, col);
		}
	}
}

static void game_2048_merge_col_bottom(struct game_2048 *g, size_t col) {
	game_2048_collapse_col_bottom(g, col);
	for (size_t row = (G2048_BOARD_SIDE - 1); row > 0; --row) {
		/* The board is already collapsed, so there's no cells after
		 * the zero */
		if (g->board[row][col] == 0)
			break;
		if (g->board[row][col] == g->board[row - 1][col]) {
			g->board[row][col] *= 2;
			g->board[row - 1][col] = 0;
			g->score += g->board[row][col];
			game_2048_collapse_col_bottom(g, col);
		}
	}
}

/* {{{1 Move functions */

void game_2048_move_left(struct game_2048 *g)
{
	for (size_t row = 0; row < G2048_BOARD_SIDE; ++row)
		game_2048_merge_row_left(g, row);
	game_2048_add_random_cell(g);
}

void game_2048_move_right(struct game_2048 *g)
{
	for (size_t row = 0; row < G2048_BOARD_SIDE; ++row)
		game_2048_merge_row_right(g, row);
	game_2048_add_random_cell(g);
}

void game_2048_move_top(struct game_2048 *g)
{
	for (size_t col = 0; col < G2048_BOARD_SIDE; ++col)
		game_2048_merge_col_top(g, col);
	game_2048_add_random_cell(g);
}

void game_2048_move_bottom(struct game_2048 *g)
{
	for (size_t col = 0; col < G2048_BOARD_SIDE; ++col)
		game_2048_merge_col_bottom(g, col);
	game_2048_add_random_cell(g);
}

/* {{{1 Game over check */

bool game_2048_is_over(struct game_2048 *g)
{
	for (size_t row = 0; row < G2048_BOARD_SIDE; ++row) {
		for (size_t col = 0; col < G2048_BOARD_SIDE; ++col) {
			if (g->board[row][col] == 0)
				return false;
		}
	}
	return true;
}

/* {{{1 Add random cell */

void game_2048_add_random_cell(struct game_2048 *g)
{
	assert(g);
	size_t x, y;

	if (game_2048_is_over(g))
		return;

	srandom(time(NULL));
	while(true) {
		x = random() % G2048_BOARD_SIDE;
		y = random() % G2048_BOARD_SIDE;

		if (g->board[x][y] == 0) {
			g->board[x][y] = (x + y) % 2 ? 2 : 4;
			break;
		}
	}
}

 /* {{{1 Initialize main structure */

void game_2048_init(struct game_2048 *g)
{
	assert(g);

	memset(g, 0, sizeof(*g));
	game_2048_add_random_cell(g);
	game_2048_add_random_cell(g);
}

