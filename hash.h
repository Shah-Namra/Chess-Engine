#pragma once

// hash.h
// zobrist hashing interface (fast board fingerprinting)

#include <cstdint>
#include "types.h"

// random keys for each (color, piece type, square)
extern uint64_t ZOBRIST_PIECES[2][6][64];

// extra key to encode whose turn it is
extern uint64_t ZOBRIST_SIDE;

// initialize random keys (call once at program start)
void init_zobrist();

// convert piece char .. color, type
// returns false if square is empty
bool piece_to_index(char p, int &color, int &type);

// compute hash from scratch by scanning board
// mainly used to verify incremental updates
uint64_t compute_hash(const class Board &board);