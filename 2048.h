#ifndef __2048_H__
#define __2048_H__

#include <inttypes.h>

#define G2048_BOARD_SIDE 4

struct game_2048 {
	uint64_t score;
	uint16_t board[G2048_BOARD_SIDE][G2048_BOARD_SIDE];
};

void game_2048_init(struct game_2048 *g);

void game_2048_move_left(struct game_2048 *g);
void game_2048_move_right(struct game_2048 *g);
void game_2048_move_top(struct game_2048 *g);
void game_2048_move_bottom(struct game_2048 *g);

void game_2048_add_random_cell(struct game_2048 *g);
bool game_2048_is_over(struct game_2048 *g);

#endif /* __2048_H__ */
