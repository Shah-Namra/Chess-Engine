#pragma once

// board.h
// Board struct (implementation in board.cpp).
// Uses char grid for now will switch to bitboards later.

#include <iostream>
#include "types.h"

// empty square marker.
const char EMPTY = '.';

struct Board
{
    // board[0] = rank 8, board[7] = rank 1.
    char squares[8][8];

    Board();

    // access helpers (only way to modify board).
    char get_piece(int row, int col) const;
    void set_piece(int row, int col, char piece);

    void print() const;
};