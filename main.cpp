// main.cpp
// tried using alpa beta pruning to optiimze the search
// not worked nodes increased instead ot=f decreasing

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

static void compare(Board &board, int depth)
{
    std::cout << "depth " << depth << ":\n";

    SearchResult mm = search_minimax(board, depth);
    std::cout << "  Minimax:    "
              << sq_to_alg(mm.best_move.from_sq) << sq_to_alg(mm.best_move.to_sq)
              << "  score=" << mm.score
              << "  nodes=" << mm.nodes << "\n";

    SearchResult ab = search_alphabeta(board, depth);
    std::cout << "  Alpha-beta: "
              << sq_to_alg(ab.best_move.from_sq) << sq_to_alg(ab.best_move.to_sq)
              << "  score=" << ab.score
              << "  nodes=" << ab.nodes << "\n";

    bool same_score = (mm.score == ab.score);
    bool fewer_nodes = (ab.nodes < mm.nodes);
    std::cout << "  Same score: " << (same_score ? "YES" : "NO")
              << "  Fewer nodes: " << (fewer_nodes ? "YES" : "NO");
    if (fewer_nodes)
        std::cout << " (" << (100 - (ab.nodes * 100 / mm.nodes)) << "% reduction)";
    std::cout << "\n\n";
}

int main()
{
    init_attack_tables();

    Board board;
    board.print();

    // compareing both search algo with diff depth to check the diff in nodes searched
    // score should be same but nodes hould be less
    // didnt worked
    compare(board, 2);
    compare(board, 3);
    compare(board, 4);

    return 0;
}
