// hash.cpp
// Zobrist hash table init and position hashing.

#include "hash.h"
#include "board.h"
#include <random>
#include <cctype>

// zoborist tables
uint64_t ZOBRIST_PIECES[2][6][64];
uint64_t ZOBRIST_SIDE;

// Maps PieceType enum values to array indices.
// p=0, k=1, b=2, r=3, q=4, k=5
static const int PIECE_INDEX[] = {0, 1, 2, 3, 4, 5};

void init_zobrist()
{
    // same random value every time seeding
    std::mt19937_64 rng(0xDEADBEEFCAFEBABEULL);

    for (int color = 0; color < 2; color++)
        for (int type = 0; type < 6; type++)
            for (int sq = 0; sq < 64; sq++)
                ZOBRIST_PIECES[color][type][sq] = rng();

    ZOBRIST_SIDE = rng();
}

bool piece_to_index(char p, int &color, int &type)
{
    if (p == '.')
        return false;

    color = std::isupper(p) ? 0 : 1; // 0 = whtie, 1 = black

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
        // XOR peice into hash
        h ^= ZOBRIST_PIECES[color][type][sq];
    }

    // include side to move
    if (board.side_to_move == BLACK)
        h ^= ZOBRIST_SIDE;

    return h;
}