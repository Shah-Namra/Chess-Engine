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

    // zpbrist hash  which updates incrementally with every move
    uint64_t zobrist_hash;

    Board();

    char get_piece(int row, int col) const;
    void set_piece(int row, int col, char piece);

    void print() const;

    UndoInfo make_move(const Move &m);                     // mpve applied and hash updated
    void unmake_move(const Move &m, const UndoInfo &undo); // move unod and restore hash

    bool load_fen(const std::string &fen);
};