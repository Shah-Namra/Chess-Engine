#pragma once

#include "board.h"
#include "types.h"

// todo: struct search infor or something
//
extern int nodes_searched;

// Result from the search: best move found and its score.
struct SearchResult
{
    Move best_move;
    int score;
    int nodes; // nodes searched to get this move and score,

    SearchResult() : best_move(0, 0, 0), score(0), nodes(0) {}
    SearchResult(Move m, int s, int n) : best_move(m), score(s), nodes(n) {}
};

// minimax
SearchResult search_minimax(Board &board, int depth);

// alpha-beta
SearchResult search_alphabeta(Board &board, int depth);

SearchResult search_tt(Board &board, int depth);

SearchResult search_ordered(Board &board, int depth);

// alpha beta + TT + move ordering + quiescence search 
SearchResult search_quiescence(Board& board, int depth);