// verifies move generation using known perft counts
// position generated from https://www.chessprogramming.org/
#include "perft.h"
#include "../core/movegen.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>

// recursive perft search
// counts all legal leaf nodes at a given depth
uint64_t perft(Board &board, int depth)
{
    // reached leaf node
    if (depth == 0)
        return 1;

    MoveList moves =
        generate_legal_moves(board,
                             board.side_to_move);

    // depth 1 = number of legal moves
    if (depth == 1)
        return moves.size();

    uint64_t nodes = 0;

    for (const Move &m : moves)
    {
        UndoInfo undo =
            board.make_move(m);

        nodes += perft(board,
                       depth - 1);

        board.unmake_move(m, undo);
    }

    return nodes;
}

// convert square index to algebraic notation
// example: 0 -> a1
static std::string sq_to_alg(int sq)
{
    std::string s;

    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));

    return s;
}

// perft divide
// shows node count for every root move
// useful for debugging movegen bugs
uint64_t perft_divide(Board &board,
                      int depth)
{
    MoveList moves =
        generate_legal_moves(board,
                             board.side_to_move);

    uint64_t total = 0;

    for (const Move &m : moves)
    {
        UndoInfo undo =
            board.make_move(m);

        uint64_t child =
            (depth <= 1)
                ? 1
                : perft(board,
                        depth - 1);

        board.unmake_move(m, undo);

        std::cout
            << sq_to_alg(m.from_sq)
            << sq_to_alg(m.to_sq)
            << ": "
            << child
            << "\n";

        total += child;
    }

    std::cout
        << "\nTotal: "
        << total
        << "\n";

    return total;
}

// test position + expected node count
struct PerftCase
{
    std::string name;
    std::string fen;

    int depth;

    uint64_t expected;
};

void run_perft_suite()
{
    std::cout
        << "\n=== Perft Test Suite ===\n\n";

    std::vector<PerftCase> cases =
    {
        // starting position
        {
            "startpos d1",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            1,
            20
        },

        {
            "startpos d2",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            2,
            400
        },

        {
            "startpos d3",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            3,
            8902
        },

        {
            "startpos d4",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            4,
            197281
        },

        {
            "startpos d5",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            5,
            4865609
        },

        // kiwipete position
        {
            "kiwipete d1",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            1,
            48
        },

        {
            "kiwipete d2",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            2,
            2039
        },

        {
            "kiwipete d3",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            3,
            97862
        },

        // endgame position
        {
            "pos3 d1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            1,
            14
        },

        {
            "pos3 d2",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            2,
            191
        },

        {
            "pos3 d3",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            3,
            2812
        },

        {
            "pos3 d4",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            4,
            43238
        },

        // promotions + pins
        {
            "pos4 d1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            1,
            6
        },

        {
            "pos4 d2",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            2,
            264
        },

        {
            "pos4 d3",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            3,
            9467
        },

        // castling + en passant tests
        {
            "pos5 d1",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            1,
            44
        },

        {
            "pos5 d2",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            2,
            1486
        },

        {
            "pos5 d3",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            3,
            62379
        },
    };

    int passed = 0;
    int total = 0;

    for (const PerftCase &c : cases)
    {
        Board board;

        if (!board.load_fen(c.fen))
        {
            std::cout
                << std::left
                << std::setw(16)
                << c.name
                << " FEN PARSE FAILED\n";

            total++;

            continue;
        }

        auto t0 =
            std::chrono::steady_clock::now();

        uint64_t got =
            perft(board,
                  c.depth);

        auto t1 =
            std::chrono::steady_clock::now();

        double ms =
            std::chrono::duration<double,
                                  std::milli>(t1 - t0)
                .count();

        bool ok =
            (got == c.expected);

        total++;

        if (ok)
            passed++;

        std::cout
            << std::left
            << std::setw(16)
            << c.name
            << (ok ? "PASS" : "FAIL")
            << "  got="
            << std::setw(12)
            << got
            << "expected="
            << std::setw(12)
            << c.expected;

        if (!ok)
        {
            long long diff =
                (long long)got -
                (long long)c.expected;

            std::cout
                << "  diff="
                << std::showpos
                << diff
                << std::noshowpos;
        }

        std::cout
            << "  ("
            << (int)ms
            << "ms)\n";
    }

    std::cout
        << "\nPerft suite: "
        << passed
        << "/"
        << total
        << " passed\n\n";
}