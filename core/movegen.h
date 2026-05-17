#pragma once

// movegen.h
// Move generation, Pseudo legal generators + legal move filter

#include "types.h"
#include "board.h"

// returns true if a square is attacked by the given side
bool is_square_attacked(const Board &board, int sq, Color attacker);
// checks if a side's king is in check
bool in_check(const Board &board, Color side);

// pseudo-legal move generators
// these do NOT check if the king is left in check
void generate_pawn_moves(const Board &board, Color side, MoveList &moves);

// knight + king
void generate_knight_moves(const Board &board, Color side, MoveList &moves);
void generate_king_moves(const Board &board, Color side, MoveList &moves);

// sliding pieces
void generate_bishop_moves(const Board &board, Color side, MoveList &moves);
void generate_rook_moves(const Board &board, Color side, MoveList &moves);
void generate_queen_moves(const Board &board, Color side, MoveList &moves);
// full move list
// generate_moves() — pseudo-legal, used for perft comparison and internally.
MoveList generate_moves(const Board &board, Color side);

// generate_legal_moves() — fully legal. Filters out any move that leaves
// the moving side's king in check. Use this in the search.
// Slightly slower than generate_moves() but correct — the search needs this
// to properly detect checkmate and stalemate.
MoveList generate_legal_moves(Board &board, Color side);