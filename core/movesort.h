#pragma once

// movesort.h
// Handles move ordering for alpha-beta search

// Order:
// 1. TT move ...best move from transposition table
// 2. Captures using MVV-LVA
// 3. Quiet move ... scores 0
//   4. History heuristic: quiet moves that historically cause cutoffs
//   5. Other quiet moves... score 0

#include "types.h"
#include "board.h"

// max search depth, sizes the killer table
constexpr int MAX_PLY = 64;

// killer move table: 2 killers per ply
// killers[ply][0] = most recent, killers[ply][1] = previous
extern Move killers[MAX_PLY][2];

// history heuristic: [color][from_sq][to_sq]
// incremented when a quiet move causes a beta cutoff
extern int history[2][64][64];

// reset killers + history (call once at start of search_iterative)
void clear_search_tables();

// record a quiet move that caused a beta cutoff
void update_killers(const Move& m, int ply);
void update_history(const Move& m, int color, int depth);

// MVV-LVA = Most Valuable Victim - Least Valuable Attacker
// Higher score = better capture.
// pawn takes queen  -> very high score
// queen takes pawn  -> low score
int mvv_lva_score(char attacker, char victim);

// Scores and sorts moves from best to worst.
// ply is current search depth from root, used to look up killers
void order_moves(MoveList& moves, const Board& board, Move tt_move, int ply);