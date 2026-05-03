// Transposition table (tt) implementation.
#include "tt.h"

// Global TT instance
TranspositionTable TT;

void TranspositionTable::store(uint64_t key, int depth, int score, int flag, Move best_move)
{
    int idx = (int)(key & (TT_SIZE - 1));
    entries[idx].key = key;
    entries[idx].depth = depth;
    entries[idx].score = score;
    entries[idx].flag = flag;
    entries[idx].best_move = best_move;
}

TTEntry *TranspositionTable::probe(uint64_t key)
{
    int idx = (int)(key & (TT_SIZE - 1));
    if (entries[idx].key == key)
        return &entries[idx];
    return nullptr;
}

void TranspositionTable::clear()
{
    for (int i = 0; i < TT_SIZE; i++)
        entries[i] = TTEntry();
}