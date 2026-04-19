// main.cpp
// Phase 9 test: make a move, print board, unmake it, print again.
// Boards must be identical. Also test a capture — captured piece must return.

#include <iostream>
#include "board.h"
#include "bitboard.h"
#include "types.h"
#include "constants.h"
#include "movegen.h"

static bool boards_equal(const Board &a, const Board &b)
{
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (a.get_piece(r, c) != b.get_piece(r, c))
                return false;
    return a.side_to_move == b.side_to_move;
}

int main()
{
    init_attack_tables();

    // --- test 1: quiet move (e2e4) ---
    {
        Board before;
        Board after;

        // e2 = sq 12, e4 = sq 28
        Move m(12, 28, FLAG_DOUBLE_PUSH);

        std::cout << "Before e2-e4:\n";
        before.print();

        UndoInfo undo = after.make_move(m);
        std::cout << "After e2-e4:\n";
        after.print();

        after.unmake_move(m, undo);
        std::cout << "After unmake:\n";
        after.print();

        if (boards_equal(before, after))
            std::cout << "Test 1 PASS: quiet move restored correctly.\n\n";
        else
            std::cout << "Test 1 FAIL: board differs after unmake.\n\n";
    }

    // --- test 2: capture (set up white rook takes black pawn) ---
    {
        Board b;
        // clear e2 pawn and put a white rook on e2, black pawn on e7 stays
        // actually let's do something simpler: put white rook on e4, black pawn on e7
        // rook on e4 = sq 28, black pawn on e7 = sq 52
        // clear the board a bit first
        b.set_piece(4, 4, 'R'); // white rook on e4
        b.set_piece(6, 4, '.'); // clear white pawn on e2 (row 6 col 4)
        // black pawn already on e7 (row 1 col 4) — leave it there

        Board before = b;

        std::cout << "Before rook captures pawn (e4xe7... actually let's do e4-e7 capture):\n";
        // e4 = sq 28, e7 = sq 52
        Move m(28, 52, FLAG_CAPTURE);

        before.print();

        UndoInfo undo = b.make_move(m);
        std::cout << "After capture:\n";
        b.print();

        b.unmake_move(m, undo);
        std::cout << "After unmake:\n";
        b.print();

        if (boards_equal(before, b))
            std::cout << "Test 2 PASS: capture and restore correct.\n\n";
        else
            std::cout << "Test 2 FAIL: board differs after unmake.\n\n";
    }

    // --- test 3: move count still 20 after make/unmake ---
    {
        Board board;
        Move m(12, 28, FLAG_DOUBLE_PUSH); // e2e4
        UndoInfo undo = board.make_move(m);
        board.unmake_move(m, undo);

        MoveList moves = generate_moves(board, WHITE);
        if (moves.size() == 20)
            std::cout << "Test 3 PASS: move count still 20 after make/unmake.\n";
        else
            std::cout << "Test 3 FAIL: expected 20 moves, got " << moves.size() << "\n";
    }

    return 0;
}

/*
// --- Phase 6/7/8 verification (commented out) ---
// compile with same command and uncomment to re-run earlier tests
*/