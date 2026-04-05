// Phase 4 — bitboard helpers and precomputed attack tables.
// main.cpp's job is still just wiring and verification.
// The phase test: print KNIGHT_ATTACKS[28] (E4) and confirm
// it shows exactly the eight squares a knight attacks from E4.

#include "board.h"
#include "bitboard.h"

int main()
{
    Board board;
    board.print();

    init_attack_tables();

    // E4 = file E (index 4) on rank 4 (index 3) = 3*8 + 4 = 28
    // A knight on E4 should attack: D2, F2, C3, G3, C5, G5, D6, F6
    std::cout << "Knight attacks from E4 (square 28):\n";
    print_bitboard(KNIGHT_ATTACKS[28]);

    return 0;
}