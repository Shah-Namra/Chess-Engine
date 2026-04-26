#pragma once

#include "board.h"
#include "types.h"

// Result from the search: best move found and its score
struct SearchResult
{
    Move best_move;
    int score;

    SearchResult() : best_move(0, 0, 0), score(0) {}
    SearchResult(Move m, int s) : best_move(m), score(s) {}
};

// gives the best move and score for curr position after searching to the given depth
SearchResult search(Board &board, int depth);