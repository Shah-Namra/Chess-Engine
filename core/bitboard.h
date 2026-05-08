#pragma once

#include <cstdint>
#include <iostream>

using Bitboard = uint64_t;

// Single-file masks used by shift functions and 1-square knight moves
const Bitboard NOT_A_FILE = 0xFEFEFEFEFEFEFEFEULL;
const Bitboard NOT_H_FILE = 0x7F7F7F7F7F7F7F7FULL;

// Two-file masks used by 2-square knight moves only
const Bitboard NOT_AB_FILE = 0xFCFCFCFCFCFCFCFCULL;
const Bitboard NOT_GH_FILE = 0x3F3F3F3F3F3F3F3FULL;

// Attack lookup tables
// Indexed by square: 0 = A1,,, 63 = H8
extern Bitboard KNIGHT_ATTACKS[64];
extern Bitboard KING_ATTACKS[64];

// Bit helpers
// Returns the index of the least significant set bit
int lsb(Bitboard bb);

// Returns the least significant set bit and removes it from bb
int pop_lsb(Bitboard &bb);

// Counts the number of set bits
int popcount(Bitboard bb);

// Shift helpers
// Move one rank up/down
inline Bitboard shift_north(Bitboard bb) { return bb << 8; }
inline Bitboard shift_south(Bitboard bb) { return bb >> 8; }

// Move left/right without wrapping around the board
inline Bitboard shift_east(Bitboard bb) { return (bb & NOT_H_FILE) << 1; }
inline Bitboard shift_west(Bitboard bb) { return (bb & NOT_A_FILE) >> 1; }

// Diagonal shifts
inline Bitboard shift_ne(Bitboard bb) { return (bb & NOT_H_FILE) << 9; }
inline Bitboard shift_nw(Bitboard bb) { return (bb & NOT_A_FILE) << 7; }
inline Bitboard shift_se(Bitboard bb) { return (bb & NOT_H_FILE) >> 7; }
inline Bitboard shift_sw(Bitboard bb) { return (bb & NOT_A_FILE) >> 9; }

// Debug
// Prints board
void print_bitboard(Bitboard bb);

// Fills the attack tables
// Call once before move generation
void init_attack_tables();