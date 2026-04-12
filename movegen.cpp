#include "movegen.h"
#include "constants.h"
#include "bitboard.h"
#include <cctype>
// movegen.cpp
// Move generation (one piece type per phase)
// pseudo-legal moves only (no check validation yet)

#include "movegen.h"
#include "constants.h"
#include "bitboard.h"
#include <cctype>

// Square index helpers, convert 0....63 into [row, col] format
static inline int sq_to_row(int sq) { return 7 - (sq / 8); }
static inline int sq_to_col(int sq) { return sq % 8; }
static inline int rc_to_sq(int row, int col) { return (7 - row) * 8 + col; }

static inline bool is_empty(const Board &board, int sq)
{
    return board.get_piece(sq_to_row(sq), sq_to_col(sq)) == EMPTY;
}

static inline char piece_at(const Board &board, int sq)
{
    return board.get_piece(sq_to_row(sq), sq_to_col(sq));
}

static inline bool is_enemy(char piece, Color side)
{
    if (piece == EMPTY)
        return false;
    // white = uppercase, black = lowercase
    return (side == WHITE) ? std::islower(piece) : std::isupper(piece);
}

static inline bool is_own(char piece, Color side)
{
    if (piece == EMPTY)
        return false;
    return (side == WHITE) ? std::isupper(piece) : std::islower(piece);
}

// pawn moves
// includes: push, double push, captures, promotions
// TODO: en passant
void generate_pawn_moves(const Board &board, Color side, MoveList &moves)
{
    const int push_dir = (side == WHITE) ? 8 : -8;
    const int start_row = (side == WHITE) ? 6 : 1;
    const int promo_row = (side == WHITE) ? 0 : 7;
    const char pawn = (side == WHITE) ? 'P' : 'p';

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            if (board.get_piece(row, col) != pawn)
                continue;

            int from_sq = rc_to_sq(row, col);
            int to_sq = from_sq + push_dir;

            // basic bounds check
            if (to_sq < 0 || to_sq > 63)
                continue;

            // forward move only if empty square
            if (is_empty(board, to_sq))
            {
                int to_row = sq_to_row(to_sq);

                if (to_row == promo_row)
                    moves.push_back(Move(from_sq, to_sq, FLAG_PROMO_QUEEN));
                else
                    moves.push_back(Move(from_sq, to_sq, FLAG_QUIET));

                // double push from starting rank
                if (row == start_row)
                {
                    int to_sq2 = to_sq + push_dir;
                    if (to_sq2 >= 0 && to_sq2 <= 63 && is_empty(board, to_sq2))
                        moves.push_back(Move(from_sq, to_sq2, FLAG_DOUBLE_PUSH));
                }
            }

            // pawn captures diagonal only also must be ene,y peice
            const int capture_dirs[2] = {push_dir - 1, push_dir + 1};
            const int col_check[2] = {col - 1, col + 1};

            for (int i = 0; i < 2; i++)
            {
                if (col_check[i] < 0 || col_check[i] > 7)
                    continue;

                int cap_sq = from_sq + capture_dirs[i];
                if (cap_sq < 0 || cap_sq > 63)
                    continue;

                char target = piece_at(board, cap_sq);
                if (!is_enemy(target, side))
                    continue;

                int to_row = sq_to_row(cap_sq);

                if (to_row == promo_row)
                    moves.push_back(Move(from_sq, cap_sq, FLAG_PROMO_QUEEN));
                else
                    moves.push_back(Move(from_sq, cap_sq, FLAG_CAPTURE));
            }

            // TODO: en passant (needs board state tracking)
        }
    }
}

// attack detection
// currently only knight king
// to Do : Sliders
bool is_square_attacked(const Board &board, int sq, Color attacker)
{
    char atk_knight = (attacker == WHITE) ? 'N' : 'n';
    Bitboard knight_atk = KNIGHT_ATTACKS[sq];

    while (knight_atk)
    {
        int from = pop_lsb(knight_atk);
        if (piece_at(board, from) == atk_knight)
            return true;
    }

    char atk_king = (attacker == WHITE) ? 'K' : 'k';
    Bitboard king_atk = KING_ATTACKS[sq];

    while (king_atk)
    {
        int from = pop_lsb(king_atk);
        if (piece_at(board, from) == atk_king)
            return true;
    }

    // TODO: sliding pieces (bishop, rook, queen)
    return false;
}

// knight moves
// simple: just iterate attack mask and filter own pieces
void generate_knight_moves(const Board &board, Color side, MoveList &moves)
{
    char knight = (side == WHITE) ? 'N' : 'n';

    for (int sq = 0; sq < 64; sq++)
    {
        if (piece_at(board, sq) != knight)
            continue;

        Bitboard atk = KNIGHT_ATTACKS[sq];

        while (atk)
        {
            int to_sq = pop_lsb(atk);
            char target = piece_at(board, to_sq);

            if (is_own(target, side))
                continue;

            int flag = is_enemy(target, side) ? FLAG_CAPTURE : FLAG_QUIET;
            moves.push_back(Move(sq, to_sq, flag));
        }
    }
}

// king moves
// also removes moves that go into check (partial attack detection)
void generate_king_moves(const Board &board, Color side, MoveList &moves)
{
    char king = (side == WHITE) ? 'K' : 'k';
    Color opp = (side == WHITE) ? BLACK : WHITE;

    for (int sq = 0; sq < 64; sq++)
    {
        if (piece_at(board, sq) != king)
            continue;

        Bitboard atk = KING_ATTACKS[sq];

        while (atk)
        {
            int to_sq = pop_lsb(atk);
            char target = piece_at(board, to_sq);

            if (is_own(target, side))
                continue;

            // skip squares attacked by opponent
            if (is_square_attacked(board, to_sq, opp))
                continue;

            int flag = is_enemy(target, side) ? FLAG_CAPTURE : FLAG_QUIET;
            moves.push_back(Move(sq, to_sq, flag));
        }
    }

    // TODO: castling (needs rights + attack checks + board state)
}

// sliding pieces (Phase 8)
void generate_bishop_moves(const Board &, Color, MoveList &) {}
void generate_rook_moves(const Board &, Color, MoveList &) {}
void generate_queen_moves(const Board &, Color, MoveList &) {}

// full move list
MoveList generate_moves(const Board &board, Color side)
{
    MoveList moves;

    generate_pawn_moves(board, side, moves);
    generate_knight_moves(board, side, moves);
    generate_king_moves(board, side, moves);
    generate_bishop_moves(board, side, moves);
    generate_rook_moves(board, side, moves);
    generate_queen_moves(board, side, moves);

    return moves;
}