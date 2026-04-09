#pragma once

// types.h
// Shared enums and move structures.

#include <vector>

enum Color
{
    WHITE,
    BLACK
};

enum PieceType
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE
};

// Basic move (can optimize/pack later if needed).
struct Move
{
    int from_sq;
    int to_sq;

    // move type flags (0=quiet, others=special).
    int flags;

    // default = quiet move.
    Move(int from, int to, int f = 0) : from_sq(from), to_sq(to), flags(f) {}
};

// list of moves (optimize later if needed).
using MoveList = std::vector<Move>;