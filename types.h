#pragma once

// types.h
// Shared enums and move structures

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

// Basic move (can optimize/pack later if needed)
struct Move
{
    int from_sq;
    int to_sq;

    // move type flags (0=quiet, others=special)
    int flags;

    // default constructor: needed for array initialization (killer table etc)
    Move() : from_sq(0), to_sq(0), flags(0) {}
    
    Move(int from, int to, int f = 0) : from_sq(from), to_sq(to), flags(f) {}
};

// list of moves (optimize later if needed).
using MoveList = std::vector<Move>;

// move flags
// it tracks captured peice and sqare where the peice caputred happen
// todo: castling, en pasasnt, underpromotions, and more
// todo:
struct UndoInfo
{
       char captured_piece; // "." no peice is captured
    int captured_sq;     // sqare where peice is captured
    int  prev_ep_square;     // ep_square before this move was made
    int  prev_castling_rights;    // castling_rgihts bitmasked before this move
};