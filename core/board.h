#pragma once

// board.h
#include <iostream>
#include <string>
#include <cstdint>
#include "types.h"

const char EMPTY = '.';

struct Board
{
    // squares[0] = rank 8, squares[7] = rank 1.
    char squares[8][8];
    Color side_to_move;

    // The square a pawn just double-pushed OVER (the skipped square)... for en passant 
    // -1 = no en passant possible..teset every move
    // Stored as a standard a1=0 .. h8=63 square index
    int ep_square;

    // Zobrist hash, updated incrementally with every move
    uint64_t zobrist_hash;

    Board();

    char get_piece(int row, int col) const;
    void set_piece(int row, int col, char piece);

    void print() const;

    UndoInfo make_move(const Move &m);
    void unmake_move(const Move &m, const UndoInfo &undo);

    bool load_fen(const std::string &fen);
};