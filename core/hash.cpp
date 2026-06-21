#include "hash.h"
#include "board.h"
#include "constants.h"
#include <random>
#include <cctype>

uint64_t ZOBRIST_PIECES[2][6][64];
uint64_t ZOBRIST_SIDE;
uint64_t ZOBRIST_EP_FILE[8];
uint64_t ZOBRIST_CASTLING[4];

void init_zobrist()
{
    std::mt19937_64 rng(0xDEADBEEFCAFEBABEULL);

    for (int color = 0; color < 2; color++)
        for (int type = 0; type < 6; type++)
            for (int sq = 0; sq < 64; sq++)
                ZOBRIST_PIECES[color][type][sq] = rng();

    ZOBRIST_SIDE = rng();

    for (int f = 0; f < 8; f++)
        ZOBRIST_EP_FILE[f] = rng();

    for (int c = 0; c < 4; c++)
        ZOBRIST_CASTLING[c] = rng();
}

bool piece_to_index(char p, int &color, int &type)
{
    if (p == '.')
        return false;
    color = std::isupper(p) ? 0 : 1;
    switch (std::toupper(p))
    {
    case 'P':
        type = 0;
        break;
    case 'N':
        type = 1;
        break;
    case 'B':
        type = 2;
        break;
    case 'R':
        type = 3;
        break;
    case 'Q':
        type = 4;
        break;
    case 'K':
        type = 5;
        break;
    default:
        return false;
    }
    return true;
}

uint64_t compute_hash(const Board &board)
{
    uint64_t h = 0;

    for (int sq = 0; sq < 64; sq++)
    {
        int row = 7 - (sq / 8);
        int col = sq % 8;
        char p = board.get_piece(row, col);
        int color, type;
        if (!piece_to_index(p, color, type))
            continue;
        h ^= ZOBRIST_PIECES[color][type][sq];
    }

    if (board.side_to_move == BLACK)
        h ^= ZOBRIST_SIDE;

    if (board.ep_square >= 0)
        h ^= ZOBRIST_EP_FILE[board.ep_square % 8];

    if (board.castling_rights & CASTLE_WK)
        h ^= ZOBRIST_CASTLING[0];
    if (board.castling_rights & CASTLE_WQ)
        h ^= ZOBRIST_CASTLING[1];
    if (board.castling_rights & CASTLE_BK)
        h ^= ZOBRIST_CASTLING[2];
    if (board.castling_rights & CASTLE_BQ)
        h ^= ZOBRIST_CASTLING[3];

    return h;
}