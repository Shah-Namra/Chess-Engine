// board.cpp
// Board method implementations.
// The struct definition lives in board.h — this file just fills in the bodies.

#include "board.h"

Board::Board()
{
    // Hardcoded starting position. Could load from FEN later but that's
    // a Phase 5 or 6 problem. Hardcoding is fine for now.
    char start[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}, // rank 8
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'}, // rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // rank 3
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'}, // rank 2
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}, // rank 1
    };

    for (int r = 0; r < 8; r++)
        for (int f = 0; f < 8; f++)
            squares[r][f] = start[r][f];
}

char Board::get_piece(int row, int col) const
{
    return squares[row][col];
}

void Board::set_piece(int row, int col, char piece)
{
    squares[row][col] = piece;
}

void Board::print() const
{
    std::cout << "\n";
    for (int rank = 0; rank < 8; rank++)
    {
        std::cout << (8 - rank) << "  ";
        for (int file = 0; file < 8; file++)
        {
            std::cout << get_piece(rank, file);
            if (file < 7)
                std::cout << ' ';
        }
        std::cout << '\n';
    }
    // TODO: add a flip option for black's perspective eventually
    std::cout << "\n   a b c d e f g h\n\n";
}