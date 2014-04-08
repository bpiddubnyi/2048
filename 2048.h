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
