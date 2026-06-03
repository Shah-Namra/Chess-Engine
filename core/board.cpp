// board.cpp
// Board method implementations

#include "board.h"
#include "constants.h"
#include "hash.h"

static inline int sq_to_row(int sq) { return 7 - (sq / 8); }
static inline int sq_to_col(int sq) { return sq % 8; }

// XOR a piece in or out of the hash at given square
// called by make_move and unmake_move... same operation because XOR is its own inverse
static inline void hash_piece(uint64_t &h, char p, int sq)
{
    int color, type;
    if (piece_to_index(p, color, type))
        h ^= ZOBRIST_PIECES[color][type][sq];
}

// XOR the EP file key in/out of hash
// Safe to call with ep == -1 
static inline void hash_ep(uint64_t &h, int ep)
{
    if (ep >= 0)
        h ^= ZOBRIST_EP_FILE[ep % 8];
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
    ep_square = -1;
    zobrist_hash = compute_hash(*this); // calc initial hash from scratch
}

char Board::get_piece(int row, int col) const
{
    return squares[row][col];
}

void Board::set_piece(int row, int col, char piece)
{
    squares[row][col] = piece;
    // NOTE: set_piece does NOT update the hash — it's used for board setup only
    // During actual play, use make_move/unmake_move which maintain the hash
    // TODO: if we ever use set_piece mid-game, we'll need to recompute hash
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
    std::cout << "  ep: " << ep_square;
    std::cout << "  hash: " << std::hex << zobrist_hash << std::dec << "\n\n";
}

UndoInfo Board::make_move(const Move &m)
{
    UndoInfo undo;
    undo.prev_ep_square = ep_square;

    char moving_piece = squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)];

    //  which square the captured piece is on
    // Normally it is m.to_sq
    // En passant case ... will be one rank behind to_sq (the pushed pawn)
    int cap_sq = m.to_sq;
    if (m.flags == FLAG_EN_PASSANT)
    {
        // White capturing en passant victim is one rank below to_sq
        // Black capturing: victim is one rank above to_sq
        cap_sq = (side_to_move == WHITE) ? (m.to_sq - 8) : (m.to_sq + 8);
    }

    undo.captured_sq = cap_sq;
    undo.captured_piece = squares[sq_to_row(cap_sq)][sq_to_col(cap_sq)];

    // remove old EP square contribution
    hash_ep(zobrist_hash, ep_square);

    // remove moving piece from source
    hash_piece(zobrist_hash, moving_piece, m.from_sq);

    // remove captured piece from hash
    if (undo.captured_piece != EMPTY)
        hash_piece(zobrist_hash, undo.captured_piece, cap_sq);

    // promotion
    if (m.flags == FLAG_PROMO_QUEEN)
        moving_piece = (side_to_move == WHITE) ? 'Q' : 'q';
    // add piece to destination in hash
    hash_piece(zobrist_hash, moving_piece, m.to_sq);

    // update ep_square based on this move
    // Only a double pawn push creates a new EP target
    if (m.flags == FLAG_DOUBLE_PUSH)
    {
        // skipped square is between from and to
        ep_square = (m.from_sq + m.to_sq) / 2;
    }
    else
    {
        ep_square = -1;
    }

    // add new EP square contribution (if any)
    hash_ep(zobrist_hash, ep_square);

    // flip side
    zobrist_hash ^= ZOBRIST_SIDE;

    // upadted board
    squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)] = EMPTY;
    if (m.flags == FLAG_EN_PASSANT)
    {
        // for EP, the captured pawn is NOT on m.to_sq 
        squares[sq_to_row(cap_sq)][sq_to_col(cap_sq)] = EMPTY;
    }
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

    // unflip side
    zobrist_hash ^= ZOBRIST_SIDE;

    // remove current EP square contribution
    hash_ep(zobrist_hash, ep_square);

    // remove piece from destination in hash
    hash_piece(zobrist_hash, squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)], m.to_sq);

    // restored capture piece in hash
    if (undo.captured_piece != EMPTY)
        hash_piece(zobrist_hash, undo.captured_piece, undo.captured_sq);

    // restored moving piece at source in hash
    hash_piece(zobrist_hash, moving_piece, m.from_sq);

    // restore EP square + its hash contribution
    ep_square = undo.prev_ep_square;
    hash_ep(zobrist_hash, ep_square);

    //  restore actual board squares
    squares[sq_to_row(m.from_sq)][sq_to_col(m.from_sq)] = moving_piece;
    squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)] = EMPTY;
    if (undo.captured_piece != EMPTY)
        squares[sq_to_row(undo.captured_sq)][sq_to_col(undo.captured_sq)] = undo.captured_piece;
}
bool Board::load_fen(const std::string &fen)
{
    // clear board first
    for (int r = 0; r < 8; r++)
    {
        for (int c = 0; c < 8; c++)
        {
            squares[r][c] = '.';
        }
        ep_square = -1;
    }
    int pos = 0;
    int row = 0;
    int col = 0;

    // parse board layout
    while (pos < (int)fen.size() &&
             fen[pos] != ' ')
    {
        char ch = fen[pos++];
        
        // next rank
        if (ch == '/')
        {
            row++;
            col = 0;
        }

        // empty squares
        else if (ch >= '1' && ch <= '8')
        {
            col += ch - '0';
        }

        // piece
        else
        {
            if (row > 7 || col > 7)
                return false;
            squares[row][col] = ch;
            col++;
        }
    }

    if (pos >= (int)fen.size())
        return false;

    pos++; // skip space

    // parse side to move
    if (pos >= (int)fen.size())
        return false;
    side_to_move = (fen[pos] == 'w') ? WHITE : BLACK;
    pos++;

    // skip space
    if (pos < (int)fen.size() && fen[pos] == ' ')
        pos++;

    // parse castling rights — SKIP for now ToDo
    while (pos < (int)fen.size() && fen[pos] != ' ')
        pos++;
    if (pos < (int)fen.size() && fen[pos] == ' ')
        pos++;

    // parse en passant target square 
    if (pos < (int)fen.size() && fen[pos] != ' ')
    {
        if (fen[pos] == '-')
        {
            ep_square = -1;
        }
        else if (pos + 1 < (int)fen.size())
        {
            int file = fen[pos] - 'a';
            int rank = fen[pos + 1] - '1';
            if (file >= 0 && file <= 7 && rank >= 0 && rank <= 7)
                ep_square = rank * 8 + file;
        }
    }

    // remaining fields (halfmove clock, fullmove number) ignored ToDo
    zobrist_hash = compute_hash(*this);

    return true;
}