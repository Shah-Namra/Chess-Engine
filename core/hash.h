#pragma once

#include <cstdint>
#include "types.h"

extern uint64_t ZOBRIST_PIECES[2][6][64];
extern uint64_t ZOBRIST_SIDE;
extern uint64_t ZOBRIST_EP_FILE[8];
extern uint64_t ZOBRIST_CASTLING[4]; // one key per right (WK, WQ, BK, BQ)

void init_zobrist();
bool piece_to_index(char p, int &color, int &type);
uint64_t compute_hash(const class Board &board);