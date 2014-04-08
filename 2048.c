#define _BSD_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "2048.h"

static void game_2048_add_random_cell(struct game_2048 *g)
{
	assert(g);
	size_t x, y;

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

void game_2048_init(struct game_2048 *g)
{
	assert(g);

	memset(g, 0, sizeof(*g));
	game_2048_add_random_cell(g);
	game_2048_add_random_cell(g);
}

