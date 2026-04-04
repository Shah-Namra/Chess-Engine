// using a char array for now just to get something visible on screen

#include <iostream>

// Piece symbols follow the standard convention:
// uppercase = white,
// lowercase = black,
// '.' = empty square.
const char EMPTY = '.';

int main()
{
    // Hardcoded starting position.
    char board[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}, // rank 8
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}, // rank 1
    };

    // Print the board with rank numbers and file letters so it actually
    // looks like a chessboard. Rank 8 is at the top, rank 1 at the bottom.
    std::cout << "\n";
    for (int rank = 0; rank < 8; rank++)
    {
        std::cout << (8 - rank) << "  ";

        for (int file = 0; file < 8; file++)
        {
            std::cout << board[rank][file];
            if (file < 7)
                std::cout << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "\n   a b c d e f g h\n\n";

    return 0;
}