// main.cpp
// Entry point. Initialises attack tables, prints the board.
// Testing code at the bottom (commented out).

#include <iostream>
#include "board.h"
#include "bitboard.h"
#include "types.h"
#include "constants.h"
#include "movegen.h"
int main()
{
    init_attack_tables();

    Board board;
    board.print();

    MoveList moves = generate_moves(board, WHITE);
    std::cout << "Total white moves from starting position: " << moves.size() << "\n";
    if (moves.size() == 20)
        std::cout << "Phase 8 passed.\n";
    else
        std::cout << "Phase 8 FAILED — expected 20.\n";

    return 0;
}

/*
// ===#6/7 verification tests ====

static std::string sq_to_alg(int sq) {
    std::string s;
    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));
    return s;
}

static void check(const char* label, int got, int expected) {
    std::cout << label << ": " << got;
    if (got == expected) std::cout << " PASS\n";
    else std::cout << " FAIL (expected " << expected << ")\n";
}

int main() {
    init_attack_tables();
    Board board;
    board.print();

    // starting position counts
    MoveList wp, wn, wk, bp, bn, bk;
    generate_pawn_moves  (board, WHITE, wp);
    generate_knight_moves(board, WHITE, wn);
    generate_king_moves  (board, WHITE, wk);
    generate_pawn_moves  (board, BLACK, bp);
    generate_knight_moves(board, BLACK, bn);
    generate_king_moves  (board, BLACK, bk);

    check("White pawns  ", wp.size(), 16);
    check("White knights", wn.size(),  4);
    check("White king   ", wk.size(),  0);
    check("Black pawns  ", bp.size(), 16);
    check("Black knights", bn.size(),  4);
    check("Black king   ", bk.size(),  0);

    // knight on A1 — corner, only 2 moves
    Board edge;
    for (int col = 0; col < 8; col++) {
        edge.set_piece(7, col, '.');
        edge.set_piece(6, col, '.');
    }
    edge.set_piece(7, 0, 'N');
    MoveList edge_moves;
    generate_knight_moves(edge, WHITE, edge_moves);
    check("Knight on A1 ", edge_moves.size(), 2);
    for (const Move& m : edge_moves)
        std::cout << "  " << sq_to_alg(m.from_sq) << " -> " << sq_to_alg(m.to_sq) << "\n";

    // knight on E4 — open board, 8 moves
    Board mid;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            mid.set_piece(r, c, '.');
    mid.set_piece(4, 4, 'N');
    MoveList mid_moves;
    generate_knight_moves(mid, WHITE, mid_moves);
    check("Knight on E4 ", mid_moves.size(), 8);

    return 0;
}
*/