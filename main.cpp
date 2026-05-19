
#include <iostream>
#include <string>
#include "core/board.h"
#include "core/bitboard.h"
#include "core/hash.h"
#include "core/search.h"
#include "bench/benchmark.h"
#include "types.h"
#include "constants.h"

static std::string sq_to_alg(int sq)
{
    std::string s;
    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));
    return s;
}

static void node_benchmark()
{
    // Calc how much each optimization reduces the search tree
    Board board;
    const int DEPTH = 5;

    std::cout << "\n Node Reduction Benchmark (depth" << DEPTH << ")\n";

    SearchResult mm = search_minimax(board, DEPTH);
    std::cout << "Minimax:" << mm.nodes << " nodes move="
              << sq_to_alg(mm.best_move.from_sq) << sq_to_alg(mm.best_move.to_sq) << "\n";

    SearchResult ab = search_alphabeta(board, DEPTH);
    int ab_pct = 100 - (ab.nodes * 100 / mm.nodes);
    std::cout << "Alpha-beta:" << ab.nodes << " nodes (" << ab_pct << "% reduction)\n";

    SearchResult tt = search_tt(board, DEPTH);
    int tt_pct = 100 - (tt.nodes * 100 / ab.nodes);
    std::cout << "AB + TT:" << tt.nodes << " nodes (" << tt_pct << "% reduction vs AB)\n";

    SearchResult ord = search_ordered(board, DEPTH);
    int ord_pct = 100 - (ord.nodes * 100 / tt.nodes);
    std::cout << "AB + TT + Ord:" << ord.nodes << " nodes (" << ord_pct << "% reduction vs TT)\n";

    int total_pct = 100 - (ord.nodes * 100 / mm.nodes);
    std::cout << "\nTotal reduction: " << total_pct << "% vs plain minimax\n";
    std::cout << "Best move: " << sq_to_alg(ord.best_move.from_sq)
              << sq_to_alg(ord.best_move.to_sq) << "score=" << ord.score << "\n";
}

int main(int argc, char *argv[])
{
    init_attack_tables();
    init_zobrist();

    std::cout << "Chess Engine Benchmark\n";

    node_benchmark();

    // accuracy puzzle
    std::cout << "\nTactical Puzzle Benchmark (depth 6)\n";
    run_benchmark("bench/puzzles.epd", 6);

    return 0;
}