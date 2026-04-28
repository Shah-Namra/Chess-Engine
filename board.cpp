// board.cpp
// Board method implementations

#include "board.h"
#include "constants.h"
#include "hash.h"

static inline int sq_to_row(int sq) { return 7 - (sq / 8); }
static inline int sq_to_col(int sq) { return sq % 8; }

// XOR a piece in or out of the hash at given square.
// called by make_move and unmake_move... same operation because XOR is its own inverse.
static inline void hash_piece(uint64_t &h, char p, int sq)
{
    int color, type;
    if (piece_to_index(p, color, type))
        h ^= ZOBRIST_PIECES[color][type][sq];
}

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
    zobrist_hash = compute_hash(*this); // calc initial hash from scratch
}

char Board::get_piece(int row, int col) const
{
    return squares[row][col];
}

void Board::set_piece(int row, int col, char piece)
{
    squares[row][col] = piece;
    // NOTE: set_piece does NOT update the hash — it's used for board setup only.
    // During actual play, use make_move/unmake_move which maintain the hash.
    // TODO: if we ever use set_piece mid-game, we'll need to recompute hash.
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
    std::cout << "\n   a b c d e f g h\n";
    std::cout << "   side: " << (side_to_move == WHITE ? "white" : "black");
    std::cout << "  hash: " << std::hex << zobrist_hash << std::dec << "\n\n";
}

UndoInfo Board::make_move(const Move &m)
{
    UndoInfo undo;
    undo.captured_sq = m.to_sq;
    undo.captured_piece = squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)];

    char moving_piece = squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)];

    // squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)] = EMPTY;

    hash_piece(zobrist_hash, moving_piece, m.from_sq);

    // remove captured piece from hash
    if (undo.captured_piece != EMPTY)
        hash_piece(zobrist_hash, undo.captured_piece, m.to_sq);

    // promotion
    if (m.flags == FLAG_PROMO_QUEEN)
        moving_piece = (side_to_move == WHITE) ? 'Q' : 'q';
    // add piece to destination in hash
    hash_piece(zobrist_hash, moving_piece, m.to_sq);

    // switch side in hash
    zobrist_hash ^= ZOBRIST_SIDE;

    // upadted board
    squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)] = EMPTY;
    squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)] = moving_piece;

    // opponent turn
    side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;

    return undo;
}

void Board::unmake_move(const Move &m, const UndoInfo &undo)
{
    // side restored
    side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;

    char moving_piece = squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)];
    // if promotion revert to pawn
    if (m.flags == FLAG_PROMO_QUEEN)
        moving_piece = (side_to_move == WHITE) ? 'P' : 'p';

    zobrist_hash ^= ZOBRIST_SIDE;

    // remove piece from destination in hash
    hash_piece(zobrist_hash, squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)], m.to_sq);

    // restored capture piece in hash
    if (undo.captured_piece != EMPTY)
        hash_piece(zobrist_hash, undo.captured_piece, undo.captured_sq);

    // restored moving piece at source in hash
    hash_piece(zobrist_hash, moving_piece, m.from_sq);

    // restore board
    squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)] = moving_piece;
    squares[sq_to_row(undo.captured_sq)][sq_to_col(undo.captured_sq)] = undo.captured_piece;
}