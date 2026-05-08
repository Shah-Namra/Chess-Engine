#pragma once

// movesort.h
// Handles move ordering for alpha-beta search

// Order:
// 1. TT move ...best move from transposition table
// 2. Captures using MVV-LVA
// 3. Quiet move ... scores 0

#include "types.h"
#include "board.h"

// MVV-LVA = Most Valuable Victim - Least Valuable Attacker
// Higher score = better capture.
// pawn takes queen  -> very high score
// queen takes pawn  -> low score
int mvv_lva_score(char attacker, char victim);

// Scores and sorts moves from best to worst.
void order_moves(MoveList &moves, const Board &board, Move tt_move);