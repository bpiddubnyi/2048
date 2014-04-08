#ifndef __2048_H__
#define __2048_H__

#include <inttypes.h>
#include <stdbool.h>

#define G2048_BOARD_SIDE 4

struct game_2048 {
	uint64_t score;
	bool     win;
	uint16_t board[G2048_BOARD_SIDE][G2048_BOARD_SIDE];
};

enum game_2048_move {
	G2048_MOVE_LEFT,
	G2048_MOVE_RIGHT,
	G2048_MOVE_TOP,
	G2048_MOVE_BOTTOM
};

void game_2048_init(struct game_2048 *g);

size_t game_2048_move(struct game_2048 *g, enum game_2048_move m);

bool game_2048_is_over(struct game_2048 *g);

#endif /* __2048_H__ */
