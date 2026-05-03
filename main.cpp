// main.cpp

#include <iostream>
#include <iomanip>
#include "core/board.h"
#include "core/bitboard.h"
#include "core/hash.h"
#include "core/movegen.h"
#include "core/eval.h"
#include "core/search.h"
#include "core/tt.h"
#include "types.h"
#include "constants.h"

static std::string sq_to_alg(int sq)
{
    std::string s;
    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));
    return s;
}

static void compare_all(Board &board, int depth)
{
    std::cout << "Depth " << depth << ":\n";

    SearchResult ab = search_alphabeta(board, depth);
    std::cout << "  Alpha-beta:    "
              << sq_to_alg(ab.best_move.from_sq) << sq_to_alg(ab.best_move.to_sq)
              << "  score=" << ab.score
              << "  nodes=" << ab.nodes << "\n";

    SearchResult tt = search_tt(board, depth);
    std::cout << "  Alpha-beta+TT: "
              << sq_to_alg(tt.best_move.from_sq) << sq_to_alg(tt.best_move.to_sq)
              << "  score=" << tt.score
              << "  nodes=" << tt.nodes << "\n";

    bool same_score = (ab.score == tt.score);
    bool fewer_nodes = (tt.nodes < ab.nodes);
    int reduction = (ab.nodes > 0) ? (100 - (tt.nodes * 100 / ab.nodes)) : 0;

    std::cout << "  Same score: " << (same_score ? "YES" : "NO")
              << "  Fewer nodes: " << (fewer_nodes ? "YES" : "NO");
    if (fewer_nodes)
        std::cout << " (" << reduction << "% reduction)";
    std::cout << "\n\n";
}

int main()
{
    init_attack_tables();
    init_zobrist();

    Board board;
    board.print();

    compare_all(board, 3);
    compare_all(board, 4);
    compare_all(board, 5);

    return 0;
}
