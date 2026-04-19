// board.cpp
// Board method implementations.
// The struct definition lives in board.h — this file just fills in the bodies.

#include "board.h"
#include "constants.h"

static inline int sq_to_row(int sq) { return 7 - (sq / 8); }
static inline int sq_to_col(int sq) { return sq % 8; }

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

    side_to_move = WHITE;
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
UndoInfo Board::make_move(const Move &m)
{
    // make_move
    UndoInfo undo;
    undo.captured_sq = m.to_sq;
    undo.captured_piece = squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)];

    char moving_piece = squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)];

    // clear the source square
    squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)] = EMPTY;

    // promotion replace pawn with queen
    // TODO: under promotions
    if (m.flags == FLAG_PROMO_QUEEN)
    {
        moving_piece = (side_to_move == WHITE) ? 'Q' : 'q';
    }

    // places the piece on destination and overwrites captured piece if any
    squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)] = moving_piece;

    // TODO: en passant ..the captured pawn is one rank behind
    // when enpassant is set,capture sq will be adjusted and pawn will also be ...

    // TODO: castling
    side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;

    return undo;
}

void Board::unmake_move(const Move &m, const UndoInfo &undo)
{
    // unmake_move put the moved piece back, restore captured piece if any.
    // need to do it perfectly

    side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;

    char moving_piece = squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)];

    // promotion? need to keep the pawn on from_sq instead of queen or whatever it was promoted to
    if (m.flags == FLAG_PROMO_QUEEN)
    {
        moving_piece = (side_to_move == WHITE) ? 'P' : 'p';
    }

    // restore from_sq
    squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)] = moving_piece;

    // restore to_sq can be empty or captured piece
    squares[sq_to_row(undo.captured_sq)][sq_to_col(undo.captured_sq)] = undo.captured_piece;
}
