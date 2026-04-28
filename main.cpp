// main.cpp
// Phase 13 test: Zobrist hash correctness.
// Compute hash from scratch, update incrementally, verify they match.
// Unmake and verify hash returns to original.

#include <iostream>
#include <iomanip>
#include "board.h"
#include "bitboard.h"
#include "hash.h"
#include "types.h"
#include "constants.h"
#include "movegen.h"

int main()
{
    init_attack_tables();
    init_zobrist();

    Board board;
    board.print();

    uint64_t original_hash = board.zobrist_hash;
    std::cout << std::hex;
    std::cout << "Initial hash (incremental): " << board.zobrist_hash << "\n";
    std::cout << "Initial hash (from scratch): " << compute_hash(board) << "\n";

    bool init_ok = (board.zobrist_hash == compute_hash(board));
    std::cout << std::dec;
    std::cout << "Test 1 " << (init_ok ? "PASS" : "FAIL")
              << ": initial hash matches scratch computation.\n\n";

    // --- make e2-e4, check hash updates ---
    Move m(12, 28, FLAG_DOUBLE_PUSH);
    UndoInfo undo = board.make_move(m);

    uint64_t after_scratch = compute_hash(board);
    bool move_ok = (board.zobrist_hash == after_scratch);
    std::cout << "After e2-e4:\n";
    std::cout << std::hex;
    std::cout << "  incremental: " << board.zobrist_hash << "\n";
    std::cout << "  from scratch: " << after_scratch << "\n";
    std::cout << std::dec;
    std::cout << "Test 2 " << (move_ok ? "PASS" : "FAIL")
              << ": hash correct after make_move.\n\n";

    // --- unmake, verify hash returns to original ---
    board.unmake_move(m, undo);

    bool unmake_ok = (board.zobrist_hash == original_hash);
    std::cout << "After unmake:\n";
    std::cout << std::hex;
    std::cout << "  incremental: " << board.zobrist_hash << "\n";
    std::cout << "  original:    " << original_hash << "\n";
    std::cout << std::dec;
    std::cout << "Test 3 " << (unmake_ok ? "PASS" : "FAIL")
              << ": hash restored after unmake_move.\n\n";

    // --- multi-move sequence ---
    // make several moves and verify hash at each step
    Move moves[] = {
        Move(12, 28, FLAG_DOUBLE_PUSH), // e2e4
        Move(52, 36, FLAG_DOUBLE_PUSH), // e7e5
        Move(6, 21, FLAG_QUIET),        // g1f3
    };

    bool sequence_ok = true;
    for (auto &mv : moves)
    {
        UndoInfo u = board.make_move(mv);
        if (board.zobrist_hash != compute_hash(board))
        {
            sequence_ok = false;
            break;
        }
        (void)u;
        // intentionally not unmaking — testing hash across multiple moves
    }
    std::cout << "Test 4 " << (sequence_ok ? "PASS" : "FAIL")
              << ": hash correct across 3-move sequence.\n";

    return 0;
}
