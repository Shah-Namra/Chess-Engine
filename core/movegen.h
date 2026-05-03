#pragma once

// movegen.h
// Move generation (implemented in phases).

#include "types.h"
#include "board.h"

// pawn moves
void generate_pawn_moves(const Board &board, Color side, MoveList &moves);

// knight + king
void generate_knight_moves(const Board &board, Color side, MoveList &moves);
void generate_king_moves(const Board &board, Color side, MoveList &moves);

// sliding pieces
void generate_bishop_moves(const Board &board, Color side, MoveList &moves);
void generate_rook_moves(const Board &board, Color side, MoveList &moves);
void generate_queen_moves(const Board &board, Color side, MoveList &moves);

// full move list
MoveList generate_moves(const Board &board, Color side);