#pragma once

// tt.h
// Transposition table cache search results indexed by Zobrist hash

// transposition table: stores already searched positions
// Each entry is 32 bytes... so 1M entries = 32MB
#include <cstdint>
#include "types.h"

// Flag values
const int TT_EXACT = 0;
const int TT_ALPHA = 1; // upper bound
const int TT_BETA = 2;  // lower bound, caused a beta cutoff

struct TTEntry
{
    uint64_t key;   //  64 bit hash to check the colisions
    int depth;      // search depth of this result
    int score;      // eval score
    int flag;       // exact, alpha, or beta
    Move best_move; // best move gives the best score

    TTEntry() : key(0), depth(0), score(0), flag(TT_EXACT), best_move(0, 0, 0) {}
};

// table size (~1M entries)
// power of 2 size lets indexing use bitwise AND instead of modulo
const int TT_SIZE = 1 << 20;

struct TranspositionTable
{
    // fixed-size table
    // collisions just overwrite old entries for now
    TTEntry entries[TT_SIZE];

    // store a search result
    void store(uint64_t key, int depth, int score, int flag, Move best_move);

    // look up a position by hash
    // returns nullptr if not found
    TTEntry *probe(uint64_t key);

    void clear();
};

// Global instance,,, one table for the whole search.
// TODO: move into a search context struct if ever want multiple parallel searches
extern TranspositionTable TT;