// movegen.cpp
// Move generation — one piece type per phase.
// pseudo-legal moves only until Phase 9 adds legality filtering.

#include "movegen.h"
#include "constants.h"
#include "bitboard.h"
#include <cctype>

// Square index helpers, convert 0....63 into [row, col] format and back

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
    // pseudo-legal: geometrically valid moves, no check filtering yet
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
            if (to_sq < 0 || to_sq > 63)
                continue;

            // single push
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

            // diagonal captures
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

            // TODO: en passant — needs ep square stored in Board state
        }
    }
}

// === Ray casting helper
// move in a direction until we hit a peice or edge of the baord
// toDo: need toimprove it

static void cast_ray(const Board &board, Color side, int from_sq, int dir_delta, int col_delta, MoveList &moves)
{
    int sq = from_sq;
    int col = sq_to_col(from_sq);

    while (true)
    {
        int next_col = col + col_delta;

        // check if wrapping around the board if yes stop
        if (col_delta != 0 && (next_col < 0 || next_col > 7))
            break;

        sq += dir_delta;
        if (sq < 0 || sq > 63)
            break;

        col = sq_to_col(sq);
        char target = piece_at(board, sq);

        // blocked by own piece
        if (is_own(target, side))
            break;
        int flag = is_enemy(target, side) ? FLAG_CAPTURE : FLAG_QUIET;
        moves.push_back(Move(from_sq, sq, flag));

        // if captured enemy peice stop
        if (flag == FLAG_CAPTURE)
            break;
    }
}

// ROok Moves
// straight lines in 4 directions till hit a piece or edge of board
void generate_rook_moves(const Board &board, Color side, MoveList &moves)
{
    char rook = (side == WHITE) ? 'R' : 'r';
    // {sq delta, col delta} — col delta tracks east/west movement for wrap detection
    const int dirs[4][2] = {{+8, 0}, {-8, 0}, {+1, +1}, {-1, -1}};

    for (int sq = 0; sq < 64; sq++)
    {
        if (piece_at(board, sq) != rook)
            continue;
        for (auto &d : dirs)
            cast_ray(board, side, sq, d[0], d[1], moves);
    }
}

// Bishoip moves
// diagonal lines in 4 direction
// ne, nw, se, sw
void generate_bishop_moves(const Board &board, Color side, MoveList &moves)
{
    char bishop = (side == WHITE) ? 'B' : 'b';
    const int dirs[4][2] = {{+9, +1}, {+7, -1}, {-7, +1}, {-9, -1}};

    for (int sq = 0; sq < 64; sq++)
    {
        if (piece_at(board, sq) != bishop)
            continue;
        for (auto &d : dirs)
            cast_ray(board, side, sq, d[0], d[1], moves);
    }
}

// queen moves
// queen = rook rays + bishop rays
void generate_queen_moves(const Board &board, Color side, MoveList &moves)
{
    // queen = rook rays + bishop rays
    char queen = (side == WHITE) ? 'Q' : 'q';
    const int dirs[8][2] = {
        {+8, 0}, {-8, 0}, {+1, +1}, {-1, -1}, // rook directions
        {+9, +1},
        {+7, -1},
        {-7, +1},
        {-9, -1} // bishop directions
    };

    for (int sq = 0; sq < 64; sq++)
    {
        if (piece_at(board, sq) != queen)
            continue;
        for (auto &d : dirs)
            cast_ray(board, side, sq, d[0], d[1], moves);
    }
}

// attack detection
// currently only knight king
// to Do : Sliders

bool is_square_attacked(const Board &board, int sq, Color attacker)
{
    // knight attacks
    char atk_knight = (attacker == WHITE) ? 'N' : 'n';
    Bitboard knight_atk = KNIGHT_ATTACKS[sq];
    while (knight_atk)
    {
        int from = pop_lsb(knight_atk);
        if (piece_at(board, from) == atk_knight)
            return true;
    }

    // king attacks
    char atk_king = (attacker == WHITE) ? 'K' : 'k';
    Bitboard king_atk = KING_ATTACKS[sq];
    while (king_atk)
    {
        int from = pop_lsb(king_atk);
        if (piece_at(board, from) == atk_king)
            return true;
    }

    // whether a square is attacked by a B/R/Q or not
    // the trick: to check if sq is attacked by R... cast a R ray FROM sq and check if it hits one of the opponents rooks or queens

    // rook/queen rays ... straight lines
    char atk_rook = (attacker == WHITE) ? 'R' : 'r';
    char atk_queen = (attacker == WHITE) ? 'Q' : 'q';
    const int rook_dirs[4][2] = {{+8, 0}, {-8, 0}, {+1, +1}, {-1, -1}};
    for (auto &d : rook_dirs)
    {
        int s = sq;
        int col = sq_to_col(s);
        while (true)
        {
            int next_col = col + d[1];
            if (d[1] != 0 && (next_col < 0 || next_col > 7))
                break;
            s += d[0];
            if (s < 0 || s > 63)
                break;
            col = sq_to_col(s);
            char p = piece_at(board, s);
            if (p == EMPTY)
                continue;
            if (p == atk_rook || p == atk_queen)
                return true;
            break; // blocked by any other piece
        }
    }

    // bishop/queen rays .. diagonals
    char atk_bishop = (attacker == WHITE) ? 'B' : 'b';
    const int bishop_dirs[4][2] = {{+9, +1}, {+7, -1}, {-7, +1}, {-9, -1}};
    for (auto &d : bishop_dirs)
    {
        int s = sq;
        int col = sq_to_col(s);
        while (true)
        {
            int next_col = col + d[1];
            if (next_col < 0 || next_col > 7)
                break;
            s += d[0];
            if (s < 0 || s > 63)
                break;
            col = sq_to_col(s);
            char p = piece_at(board, s);
            if (p == EMPTY)
                continue;
            if (p == atk_bishop || p == atk_queen)
                return true;
            break;
        }
    }

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
            if (is_square_attacked(board, to_sq, opp))
                continue;
            int flag = is_enemy(target, side) ? FLAG_CAPTURE : FLAG_QUIET;
            moves.push_back(Move(sq, to_sq, flag));
        }
    }
    // TODO: castling (needs rights + attack checks + board state)
}

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