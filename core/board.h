#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include "types.h"

const char EMPTY = '.';

struct Board
{
    char squares[8][8]; // squares[0] = rank 8, squares[7] = rank 1
    Color side_to_move;
    int ep_square;       // a1=0..h8=63, -1 = none
    int castling_rights; // bitmask: CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ
    uint64_t zobrist_hash;

    Board();

    char get_piece(int row, int col) const;
    void set_piece(int row, int col, char piece);

    void print() const;

    UndoInfo make_move(const Move &m);
    void unmake_move(const Move &m, const UndoInfo &undo);

    bool load_fen(const std::string &fen);
};