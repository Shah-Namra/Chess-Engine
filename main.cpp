
#include <iostream>
#include "core/board.h"
#include "core/bitboard.h"
#include "core/hash.h"
#include "core/search.h"
#include "types.h"
#include "constants.h"

static std::string sq_to_alg(int sq)
{
    std::string s;
    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));
    return s;
}

int main()
{
    init_attack_tables();
    init_zobrist();

    Board board;
    board.print();

    const int DEPTH = 5;
    std::cout << "Searching to depth " << DEPTH << "...\n";
    SearchResult result = search_ordered(board, DEPTH);

    std::cout << "Best move: "
              << sq_to_alg(result.best_move.from_sq)
              << sq_to_alg(result.best_move.to_sq)
              << "  score=" << result.score
              << "  nodes=" << result.nodes << "\n";

    return 0;
}