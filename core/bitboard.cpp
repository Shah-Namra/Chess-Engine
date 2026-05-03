// bitboard.cpp
// Helper implementations

#include "bitboard.h"

// Attack tables (defined here).
Bitboard KNIGHT_ATTACKS[64];
Bitboard KING_ATTACKS[64];

int lsb(Bitboard bb)
{
    // index of lowest set bit (bb must non-zero)
    return __builtin_ctzll(bb);
}

int pop_lsb(Bitboard &bb)
{
    int sq = lsb(bb);
    bb &= bb - 1; // remove lowest bit
    return sq;
}

int popcount(Bitboard bb)
{
    // count bits using builtin function
    return __builtin_popcountll(bb);
}

void print_bitboard(Bitboard bb)
{
    std::cout << "\n";
    for (int rank = 7; rank >= 0; rank--)
    {
        std::cout << (rank + 1) << "  ";
        for (int file = 0; file < 8; file++)
        {
            int sq = rank * 8 + file;
            std::cout << ((bb >> sq) & 1);
            if (file < 7)
                std::cout << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "\n   a b c d e f g h\n\n";
}

void init_attack_tables()
{
    // precompute knight and king moves.

    for (int sq = 0; sq < 64; sq++)
    {
        Bitboard b = (Bitboard)1 << sq;

        // knight moves 8 possible moves, but some may be off-board
        KNIGHT_ATTACKS[sq] =
            ((b << 17) & NOT_A_FILE) |  // 2N 1E
            ((b << 15) & NOT_GH_FILE) | // 2N 1W
            ((b << 10) & NOT_A_FILE) |  // 1N 2E
            ((b << 6) & NOT_GH_FILE) |  // 1N 2W
            ((b >> 6) & NOT_AB_FILE) |  // 1S 2E
            ((b >> 10) & NOT_H_FILE) |  // 1S 2W
            ((b >> 15) & NOT_AB_FILE) | // 2S 1E
            ((b >> 17) & NOT_H_FILE);   // 2S 1W
        // king moves 8 possible moves removed the off board ones
        KING_ATTACKS[sq] =
            shift_north(b) |
            shift_south(b) |
            shift_east(b) |
            shift_west(b) |
            shift_ne(b) |
            shift_nw(b) |
            shift_se(b) |
            shift_sw(b);
    }
}