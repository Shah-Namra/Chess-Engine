#pragma once

// benchmark.h
// simple benchmark system for testing engine strength on puzzles

#include <string>
#include <vector>

struct PuzzleResult
{
    std::string fen;      // puzzle position
    std::string expected; // expected best move
    std::string found;    // engine move

    bool correct; // did engine solve it?

    int nodes; // nodes searched
};

// run all puzzles from an EPD file
// prints accuracy and search statistics
void run_benchmark(const std::string &epd_path, int depth);