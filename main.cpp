
#include <iostream>

// Piece symbols:
// uppercase = white,
// lowercase = black,
// '.' = empty.
const char EMPTY = '.';

struct Board
{
    char squares[8][8];

    // Sets up the standard chess starting position.
    Board()
    {
        char start[8][8] = {
            {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}, // rank 8
            {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
            {'.', '.', '.', '.', '.', '.', '.', '.'},
            {'.', '.', '.', '.', '.', '.', '.', '.'},
            {'.', '.', '.', '.', '.', '.', '.', '.'},
            {'.', '.', '.', '.', '.', '.', '.', '.'},
            {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
            {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}, // rank 1
        };

        for (int r = 0; r < 8; r++)
            for (int f = 0; f < 8; f++)
                squares[r][f] = start[r][f];
    }

    char get_piece(int row, int col) const
    {
        return squares[row][col];
    }

    void set_piece(int row, int col, char piece)
    {
        squares[row][col] = piece;
    }

    // Prints the board the way a chess player expects to see it:
    // rank 8 at the top, rank 1 at the bottom, files labeled a-h.
    void print() const
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
        // TODO: the print function is going to need a "flip" option eventually
        std::cout << "\n   a b c d e f g h\n\n";
    }
};

int main()
{
    Board board;
    board.print();
    return 0;
}