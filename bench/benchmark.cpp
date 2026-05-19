// benchmark.cpp
// runs puzzle benchmarks on the engine

#include "benchmark.h"
#include "../core/board.h"
#include "../core/bitboard.h"
#include "../core/hash.h"
#include "../core/movegen.h"
#include "../core/search.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

// convert square index -> algebraic notation
// example: 28 -> "e4"
static std::string sq_to_alg(int sq)
{
    std::string s;

    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));

    return s;
}

// convert algebraic notation -> square index
// example: "e4" -> 28
static int alg_to_sq(const std::string &s)
{
    if (s.size() < 2)
        return -1;

    int file = s[0] - 'a';
    int rank = s[1] - '1';

    if (file < 0 || file > 7 ||
        rank < 0 || rank > 7)
        return -1;

    return rank * 8 + file;
}

// checks if algebraic move matches Move object
// example: "e2e4"
static bool move_matches(const std::string &alg,
                         const Move &m)
{
    if (alg.size() < 4)
        return false;

    std::string from_str = alg.substr(0, 2);
    std::string to_str = alg.substr(2, 2);

    int from = alg_to_sq(from_str);
    int to = alg_to_sq(to_str);

    if (from < 0 || to < 0)
        return false;

    return m.from_sq == from &&
           m.to_sq == to;
}

// parse "bm" field into list of moves
static std::vector<std::string>
parse_bm(const std::string &bm_field)
{
    std::vector<std::string> moves;

    std::istringstream ss(bm_field);

    std::string token;

    while (ss >> token)
    {
        // remove trailing punctuation
        while (!token.empty() &&
               (token.back() == ';' ||
                token.back() == ','))
        {
            token.pop_back();
        }

        if (!token.empty())
            moves.push_back(token);
    }

    return moves;
}

void run_benchmark(const std::string &epd_path, int depth)
{
    std::ifstream file(epd_path);

    if (!file.is_open())
    {
        std::cerr << "Could not open puzzle file: "
                  << epd_path << "\n";
        return;
    }

    int total = 0;
    int correct = 0;

    long long total_nodes = 0;
    double total_ms = 0;

    std::string line;

    while (std::getline(file, line))
    {
        // skip empty lines/comments
        if (line.empty() || line[0] == '#')
            continue;

        // split FEN and best move
        size_t bm_pos = line.find(" bm ");

        if (bm_pos == std::string::npos)
            continue;

        std::string fen =
            line.substr(0, bm_pos);

        std::string bm_rest =
            line.substr(bm_pos + 4);

        std::vector<std::string> best_moves =
            parse_bm(bm_rest);

        if (best_moves.empty())
            continue;

        Board board;

        // load position
        if (!board.load_fen(fen))
        {
            std::cerr << "Failed to parse FEN: "
                      << fen << "\n";
            continue;
        }
        const double TIME_LIMIT_MS = 2000.0;

        // run search + measure time
        auto t0 = std::chrono::steady_clock::now();
        auto t0 = std::chrono::steady_clock::now();
        SearchResult result = search_ordered(board, depth); 
        auto t1 = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        total_ms += ms;
        total_nodes += result.nodes;

        // convert engine move to text
        std::string played =
            sq_to_alg(result.best_move.from_sq) +
            sq_to_alg(result.best_move.to_sq);

        bool found = false;

        // compare against acceptable moves
        for (const std::string &bm : best_moves)
        {
            if (played == bm ||
                move_matches(bm,
                             result.best_move))
            {
                found = true;
                break;
            }
        }

        total++;

        if (found)
            correct++;

        std::cout
            << (found ? "PASS" : "FAIL")
            << "  " << played
            << " (expected: "
            << best_moves[0] << ") "
            << "nodes=" << result.nodes
            << "  " << (int)ms << "ms\n";
    }

    // final stats
    std::cout << "\n";
    std::cout << "Puzzles: " << total << "\n";
    std::cout << "Correct: " << correct << "\n";
    std::cout << "Accuracy: " << (total > 0 ? (correct * 100 / total) : 0) << "%\n";
    std::cout << "Total nodes: " << total_nodes << "\n";
    std::cout << "Total time: " << (int)total_ms << "ms\n";

    if (total > 0)
    {
        std::cout << "Avg nodes/puzzle: " << total_nodes / total << "\n";
    }
}