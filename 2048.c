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

#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "2048.h"

/* {{{1 Collapse functions */

/* {{{2 Row collapse */

static size_t game_2048_collapse_row_left(struct game_2048 *g, size_t row)
{
	ssize_t first_zero = -1;
	size_t cells_moved = 0;
	for (size_t col = 0; col < G2048_BOARD_SIDE; ++col) {
		if (g->board[row][col] == 0) {
			if (first_zero == -1)
				first_zero = col;
			continue;
		}

		if (first_zero > -1) {
			g->board[row][first_zero++] = g->board[row][col];
			g->board[row][col] = 0;
			++cells_moved;
		}
	}
	return cells_moved;
}

static size_t game_2048_collapse_row_right(struct game_2048 *g, size_t row)
{
	ssize_t first_zero = -1;
	size_t cells_moved = 0;
	for (ssize_t col = G2048_BOARD_SIDE - 1; col >= 0; --col) {
		if (g->board[row][col] == 0) {
			if (first_zero == -1)
				first_zero = col;
			continue;
		}

		if (first_zero > -1) {
			g->board[row][first_zero--] = g->board[row][col];
			g->board[row][col] = 0;
			++cells_moved;
		}
	}
	return cells_moved;
}

/* {{{2 Column collapse */

static size_t game_2048_collapse_col_top(struct game_2048 *g, size_t col)
{
	ssize_t first_zero = -1;
	size_t cells_moved = 0;
	for (ssize_t row = 0; row < G2048_BOARD_SIDE; ++row) {
		if (g->board[row][col] == 0) {
			if (first_zero == -1)
				first_zero = row;
			continue;
		}

		if (first_zero > -1) {
			g->board[first_zero++][col] = g->board[row][col];
			g->board[row][col] = 0;
			++cells_moved;
		}
	}
	return cells_moved;
}

static size_t game_2048_collapse_col_bottom(struct game_2048 *g, size_t col)
{
	ssize_t first_zero = -1;
	size_t cells_moved = 0;
	for (ssize_t row = G2048_BOARD_SIDE - 1; row >= 0; --row) {
		if (g->board[row][col] == 0) {
			if (first_zero == -1)
				first_zero = row;
			continue;
		}

		if (first_zero > -1) {
			g->board[first_zero--][col] = g->board[row][col];
			g->board[row][col] = 0;
			++cells_moved;
		}
	}
	return cells_moved;
}

/* {{{1 Merge functions */

/* {{{2 Row merge */

static size_t game_2048_merge_row_left(struct game_2048 *g, size_t row) {
	size_t cells_moved = 0;
	cells_moved += game_2048_collapse_row_left(g, row);
	for (size_t col = 0; col < (G2048_BOARD_SIDE - 1); ++col) {
		/* The board is already collapsed, so there's no cells after
		 * the zero */
		if (g->board[row][col] == 0)
			break;
		if (g->board[row][col] == g->board[row][col + 1]) {
			g->board[row][col] *= 2;
			g->board[row][col + 1] = 0;
			g->free_cells++;
			g->score += g->board[row][col];
			
			if (g->board[row][col] == 2048)
				g->win = true;

			++cells_moved;
			cells_moved += game_2048_collapse_row_left(g, row);
		}
	}
	return cells_moved;
}

static size_t game_2048_merge_row_right(struct game_2048 *g, size_t row) {
	size_t cells_moved = 0;
	cells_moved += game_2048_collapse_row_right(g, row);
	for (size_t col = (G2048_BOARD_SIDE - 1); col > 0; --col) {
		/* The board is already collapsed, so there's no cells after
		 * the zero */
		if (g->board[row][col] == 0)
			break;
		if (g->board[row][col] == g->board[row][col - 1]) {
			g->board[row][col] *= 2;
			g->board[row][col - 1] = 0;
			g->free_cells++;
			g->score += g->board[row][col];
			
			if (g->board[row][col] == 2048)
				g->win = true;

			++cells_moved;
			cells_moved += game_2048_collapse_row_right(g, row);
		}
	}
	return cells_moved;
}

/* {{{2 Column merge */

static size_t game_2048_merge_col_top(struct game_2048 *g, size_t col) {
	size_t cells_moved = 0;
	cells_moved += game_2048_collapse_col_top(g, col);
	for (size_t row = 0; row < (G2048_BOARD_SIDE - 1); ++row) {
		/* The board is already collapsed, so there's no cells after
		 * the zero */
		if (g->board[row][col] == 0)
			break;
		if (g->board[row][col] == g->board[row + 1][col]) {
			g->board[row][col] *= 2;
			g->board[row + 1][col] = 0;
			g->free_cells++;
			g->score += g->board[row][col];
			
			if (g->board[row][col] == 2048)
				g->win = true;

			++cells_moved;
			cells_moved += game_2048_collapse_col_top(g, col);
		}
	}
	return cells_moved;
}

static size_t game_2048_merge_col_bottom(struct game_2048 *g, size_t col) {
	size_t cells_moved = 0;
	cells_moved += game_2048_collapse_col_bottom(g, col);
	for (size_t row = (G2048_BOARD_SIDE - 1); row > 0; --row) {
		/* The board is already collapsed, so there's no cells after
		 * the zero */
		if (g->board[row][col] == 0)
			break;
		if (g->board[row][col] == g->board[row - 1][col]) {
			g->board[row][col] *= 2;
			g->board[row - 1][col] = 0;
			g->free_cells++;
			g->score += g->board[row][col];
			
			if (g->board[row][col] == 2048)
				g->win = true;

			++cells_moved;
			cells_moved += game_2048_collapse_col_bottom(g, col);
		}
	}
	return cells_moved;
}

/* {{{1 Add random cell */

void game_2048_add_random_cell(struct game_2048 *g)
{
	int cellno;

	assert(g);

	if (game_2048_is_over(g))
		return;

	cellno = random() % g->free_cells;

	for (size_t x = 0; x < G2048_BOARD_SIDE; ++x) {
		for (size_t y = 0; y < G2048_BOARD_SIDE; ++y) {
			if (g->board[x][y] == 0 && cellno-- == 0) {
				g->board[x][y] = (random() % 2 + 1) * 2;
				break;
			}
		}
	}

	g->free_cells--;
}

/* {{{1 Move */

size_t game_2048_move(struct game_2048 *g, enum game_2048_move m)
{
	size_t cells_moved = 0;

	assert(g);
	
	for (size_t i = 0; i < G2048_BOARD_SIDE; ++i) {
		switch(m) {
		case G2048_MOVE_LEFT:
			cells_moved += game_2048_merge_row_left(g, i);
			break;
		case G2048_MOVE_RIGHT:
			cells_moved += game_2048_merge_row_right(g, i);
			break;
		case G2048_MOVE_TOP:
			cells_moved += game_2048_merge_col_top(g, i);
			break;
		case G2048_MOVE_BOTTOM:
			cells_moved += game_2048_merge_col_bottom(g, i);
			break;
		}
	}
	if (cells_moved)
		game_2048_add_random_cell(g);

	return cells_moved;
}

/* {{{1 Game over check */

static bool game_2048_cell_is_mergeable(struct game_2048 *g, size_t row, 
					size_t col)
{
	if (col > 0 && g->board[row][col -1] == g->board[row][col])
		return true;

	if (row > 0 && g->board[row - 1][col] == g->board[row][col])
		return true;

	if (col < G2048_BOARD_SIDE - 1 && (g->board[row][col + 1]
					  == g->board[row][col]))
		return true;

	if (row < G2048_BOARD_SIDE - 1 && (g->board[row + 1][col]
					   == g->board[row][col]))
		return true;

	return false;
}

bool game_2048_is_over(struct game_2048 *g)
{
	for (size_t row = 0; row < G2048_BOARD_SIDE; ++row) {
		for (size_t col = 0; col < G2048_BOARD_SIDE; ++col) {
			if (g->board[row][col] == 0)
				return false;
			if (game_2048_cell_is_mergeable(g, row, col))
				return false;
		}
	}
	return true;
}

/* {{{1 Initialize main structure */

void game_2048_init(struct game_2048 *g)
{
	assert(g);

	srandom(time(NULL));

	memset(g, 0, sizeof(*g));
	g->free_cells = G2048_BOARD_SIDE * G2048_BOARD_SIDE;

	game_2048_add_random_cell(g);
	game_2048_add_random_cell(g);
}

