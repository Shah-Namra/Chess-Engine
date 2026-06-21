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
static inline char promo_piece(int flag, Color side)
{
    bool white = (side == WHITE);
    switch (flag)
    {
    case FLAG_PROMO_QUEEN:
        return white ? 'Q' : 'q';
    case FLAG_PROMO_ROOK:
        return white ? 'R' : 'r';
    case FLAG_PROMO_BISHOP:
        return white ? 'B' : 'b';
    case FLAG_PROMO_KNIGHT:
        return white ? 'N' : 'n';
    default:
        return 0;
    }
}

static inline bool is_promotion(int flag)
{
    return flag == FLAG_PROMO_QUEEN || flag == FLAG_PROMO_ROOK ||
           flag == FLAG_PROMO_BISHOP || flag == FLAG_PROMO_KNIGHT;
}

// XOR castling rights into/out of the hash.
// To CHANGE rights: hash out old, mutate, hash in new.
static inline void hash_castling(uint64_t &h, int rights)
{
    if (rights & CASTLE_WK)
        h ^= ZOBRIST_CASTLING[0];
    if (rights & CASTLE_WQ)
        h ^= ZOBRIST_CASTLING[1];
    if (rights & CASTLE_BK)
        h ^= ZOBRIST_CASTLING[2];
    if (rights & CASTLE_BQ)
        h ^= ZOBRIST_CASTLING[3];
}

// corner square constants for rook-moved / rook-captured rights updates
// a1=0, h1=7, a8=56, h8=63 in A1=0 indexing
static const int SQ_A1 = 0, SQ_H1 = 7, SQ_A8 = 56, SQ_H8 = 63;
static const int SQ_E1 = 4, SQ_E8 = 60;

// Updating castling rights based on what moved/got captured at these squares
// if a piece leaves or comes at a relevant square... the appropriate rights drop
static inline int updated_rights(int rights, int from_sq, int to_sq)
{
    // If the king moves (or castles), lose both rights for that color.
    if (from_sq == SQ_E1)
        rights &= ~(CASTLE_WK | CASTLE_WQ);
    if (from_sq == SQ_E8)
        rights &= ~(CASTLE_BK | CASTLE_BQ);

    // If a rook leaves its starting square, lose that right.
    if (from_sq == SQ_H1)
        rights &= ~CASTLE_WK;
    if (from_sq == SQ_A1)
        rights &= ~CASTLE_WQ;
    if (from_sq == SQ_H8)
        rights &= ~CASTLE_BK;
    if (from_sq == SQ_A8)
        rights &= ~CASTLE_BQ;

    // If a rook gets captured on its starting square, lose that right.
    if (to_sq == SQ_H1)
        rights &= ~CASTLE_WK;
    if (to_sq == SQ_A1)
        rights &= ~CASTLE_WQ;
    if (to_sq == SQ_H8)
        rights &= ~CASTLE_BK;
    if (to_sq == SQ_A8)
        rights &= ~CASTLE_BQ;

    return rights;
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
    castling_rights = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
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
    std::cout << "  castling: " << castling_rights;
    std::cout << "  hash: " << std::hex << zobrist_hash << std::dec << "\n\n";
}

UndoInfo Board::make_move(const Move &m)
{
    UndoInfo undo;
    undo.prev_ep_square = ep_square;
    undo.prev_castling_rights = castling_rights;

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
    hash_castling(zobrist_hash, castling_rights);

    // remove moving piece from source
    hash_piece(zobrist_hash, moving_piece, m.from_sq);

    // remove captured piece from hash
    if (undo.captured_piece != EMPTY)
        hash_piece(zobrist_hash, undo.captured_piece, cap_sq);

    // promotion
    if (is_promotion(m.flags))
        moving_piece = promo_piece(m.flags, side_to_move);

    // add piece to destination in hash
    hash_piece(zobrist_hash, moving_piece, m.to_sq);

    // castling: also move the rook in hash + on board
    if (m.flags == FLAG_CASTLE_KINGSIDE)
    {
        // king moves e->g; rook moves h->f on same rank
        int rook_from = (side_to_move == WHITE) ? SQ_H1 : SQ_H8;
        int rook_to = (side_to_move == WHITE) ? 5 : 61; // f1=5, f8=61
        char rook = squares[sq_to_row(rook_from)][sq_to_col(rook_from)];
        hash_piece(zobrist_hash, rook, rook_from);
        hash_piece(zobrist_hash, rook, rook_to);
        squares[sq_to_row(rook_from)][sq_to_col(rook_from)] = EMPTY;
        squares[sq_to_row(rook_to)][sq_to_col(rook_to)] = rook;
    }
    else if (m.flags == FLAG_CASTLE_QUEENSIDE)
    {
        // king moves e->c; rook moves a->d on same rank
        int rook_from = (side_to_move == WHITE) ? SQ_A1 : SQ_A8;
        int rook_to = (side_to_move == WHITE) ? 3 : 59; // d1=3, d8=59
        char rook = squares[sq_to_row(rook_from)][sq_to_col(rook_from)];
        hash_piece(zobrist_hash, rook, rook_from);
        hash_piece(zobrist_hash, rook, rook_to);
        squares[sq_to_row(rook_from)][sq_to_col(rook_from)] = EMPTY;
        squares[sq_to_row(rook_to)][sq_to_col(rook_to)] = rook;
    }

    // update castling rights (based on from/to squares)
    castling_rights = updated_rights(castling_rights, m.from_sq, m.to_sq);

    //  update ep_square (only set by double push)
    if (m.flags == FLAG_DOUBLE_PUSH)
        ep_square = (m.from_sq + m.to_sq) / 2;
    else
        ep_square = -1;

    // add new EP square contribution (if any) +castling rgihts
    hash_ep(zobrist_hash, ep_square);
    hash_castling(zobrist_hash, castling_rights);

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
    if (is_promotion(m.flags))
        moving_piece = (side_to_move == WHITE) ? 'P' : 'p';

    // unflip side
    zobrist_hash ^= ZOBRIST_SIDE;

    // remove current EP square contribution
    hash_ep(zobrist_hash, ep_square);
    hash_castling(zobrist_hash, castling_rights);

    // remove piece from destination in hash
    hash_piece(zobrist_hash, squares[sq_to_row(m.to_sq)][sq_to_col(m.to_sq)], m.to_sq);

    // restored capture piece in hash
    if (undo.captured_piece != EMPTY)
        hash_piece(zobrist_hash, undo.captured_piece, undo.captured_sq);

    // restored moving piece at source in hash
    hash_piece(zobrist_hash, moving_piece, m.from_sq);

    // castling: undo the rook move
    if (m.flags == FLAG_CASTLE_KINGSIDE)
    {
        int rook_from = (side_to_move == WHITE) ? SQ_H1 : SQ_H8;
        int rook_to = (side_to_move == WHITE) ? 5 : 61;
        char rook = squares[sq_to_row(rook_to)][sq_to_col(rook_to)];
        hash_piece(zobrist_hash, rook, rook_to);
        hash_piece(zobrist_hash, rook, rook_from);
        squares[sq_to_row(rook_to)][sq_to_col(rook_to)] = EMPTY;
        squares[sq_to_row(rook_from)][sq_to_col(rook_from)] = rook;
    }
    else if (m.flags == FLAG_CASTLE_QUEENSIDE)
    {
        int rook_from = (side_to_move == WHITE) ? SQ_A1 : SQ_A8;
        int rook_to = (side_to_move == WHITE) ? 3 : 59;
        char rook = squares[sq_to_row(rook_to)][sq_to_col(rook_to)];
        hash_piece(zobrist_hash, rook, rook_to);
        hash_piece(zobrist_hash, rook, rook_from);
        squares[sq_to_row(rook_to)][sq_to_col(rook_to)] = EMPTY;
        squares[sq_to_row(rook_from)][sq_to_col(rook_from)] = rook;
    }

    //  restore EP + castling rights from undo
    ep_square = undo.prev_ep_square;
    castling_rights = undo.prev_castling_rights;

    //  hash: add restored EP + restored castling rights
    hash_ep(zobrist_hash, ep_square);
    hash_castling(zobrist_hash, castling_rights);

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
    }
    ep_square = -1;
    castling_rights = 0;
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
    if (pos < (int)fen.size() && fen[pos] == ' ')
        pos++;

    // parse castling rights
    while (pos < (int)fen.size() && fen[pos] != ' ')
    {
        char ch = fen[pos++];
        switch (ch)
        {
        case 'K':
            castling_rights |= CASTLE_WK;
            break;
        case 'Q':
            castling_rights |= CASTLE_WQ;
            break;
        case 'k':
            castling_rights |= CASTLE_BK;
            break;
        case 'q':
            castling_rights |= CASTLE_BQ;
            break;
        case '-':
            break;
        default:
            break;
        }
    }
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