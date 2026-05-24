#pragma once

// perft.h
// perft = move generation test
// used to verify legal move generation
// against known correct node counts

#include <cstdint>
#include <string>

#include "../core/board.h"

// count leaf nodes from current position
uint64_t perft(Board& board, int depth);

// perft divide
// prints node count for every root move
// useful for debugging movegen bugs
uint64_t perft_divide(Board& board, int depth);

// run standard perft test positions
void run_perft_suite();