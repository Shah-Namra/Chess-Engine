// main.cpp

#include <iostream>
#include "board.h"
#include "bitboard.h"
#include "types.h"
#include "constants.h"
#include "movegen.h"
#include "eval.h"
#include "search.h"

static std::string sq_to_alg(int sq)
{
    std::string s;
    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));
    return s;
}

static void print_move(const char *label, const SearchResult &r)
{
    std::cout << label << ": "
              << sq_to_alg(r.best_move.from_sq) << " -> "
              << sq_to_alg(r.best_move.to_sq)
              << "  (score: " << r.score << ")\n";
}

int main()
{
    init_attack_tables();

    //  Test 1: depth 1 material grab
    // White queen can take black queen for free. Engine must take it.
    {
        Board b;
        for (int r = 0; r < 8; r++)
            for (int c = 0; c < 8; c++)
                b.set_piece(r, c, '.');
        b.set_piece(7, 6, 'K'); // Kg1
        b.set_piece(7, 3, 'Q'); // Qd1
        b.set_piece(0, 6, 'k'); // kg8
        b.set_piece(0, 3, 'q'); // qd8 should be captured
        b.side_to_move = WHITE;

        std::cout << "Test 1 — free queen capture:\n";
        b.print();
        SearchResult r = search(b, 1);
        print_move("Engine", r);

        bool took_queen = (r.best_move.from_sq == 3 && r.best_move.to_sq == 59);
        std::cout << (took_queen ? "PASS: took free queen.\n" : "FAIL: missed free queen.\n") << "\n";
    }

    //  Test 2: depth 2 don't hang the queen
    // White Q can take a pawn, but the pawn is defended by black q.
    // At depth 1 engine might take the pawn.
    // ButAt depth 2 it should see it loses a queen.
    {
        Board b;
        for (int r = 0; r < 8; r++)
            for (int c = 0; c < 8; c++)
                b.set_piece(r, c, '.');
        b.set_piece(7, 6, 'K'); // Kg1
        b.set_piece(7, 3, 'Q'); // Qd1
        b.set_piece(0, 6, 'k'); // kg8
        b.set_piece(4, 3, 'p'); // pd4 pawn white queen can take
        b.set_piece(3, 3, 'q'); // qd5 black queen defends d4
        b.side_to_move = WHITE;

        std::cout << "Test 2: don't hang queen (depth 2):\n";
        b.print();

        SearchResult r1 = search(b, 1);
        SearchResult r2 = search(b, 2);
        print_move("Depth 1", r1);
        print_move("Depth 2", r2);

        // at depth 2, taking d4 (sq 27) should be avoided
        bool depth1_greedy = (r1.best_move.to_sq == 27);
        bool depth2_avoids = (r2.best_move.to_sq != 27);

        std::cout << (depth1_greedy ? "Depth 1 takes pawn (greedy, as expected).\n"
                                    : "Depth 1 did not take pawn.\n");
        std::cout << (depth2_avoids ? "PASS: depth 2 avoids losing queen.\n"
                                    : "FAIL: depth 2 still hangs queen.\n")
                  << "\n";
    }

    //  Test 3: depth 2 regresion move count unchanged after search
    {
        Board b;
        search(b, 2);
        MoveList moves = generate_moves(b, WHITE);
        std::cout << "Test 3 — board unchanged after search: ";
        std::cout << (moves.size() == 20 ? "PASS (still 20 moves)\n" : "FAIL\n");
    }

    return 0;
}
