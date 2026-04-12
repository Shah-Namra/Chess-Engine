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

inline Bitboard shift_north(Bitboard b) { return b << 8; }
inline Bitboard shift_south(Bitboard b) { return b >> 8; }
inline Bitboard shift_east(Bitboard b) { return (b & NOT_H_FILE) << 1; }
inline Bitboard shift_west(Bitboard b) { return (b & NOT_A_FILE) >> 1; }
inline Bitboard shift_ne(Bitboard b) { return (b & NOT_H_FILE) << 9; }
inline Bitboard shift_nw(Bitboard b) { return (b & NOT_A_FILE) << 7; }
inline Bitboard shift_se(Bitboard b) { return (b & NOT_H_FILE) >> 7; }
inline Bitboard shift_sw(Bitboard b) { return (b & NOT_A_FILE) >> 9; }

extern Bitboard KNIGHT_ATTACKS[64];
extern Bitboard KING_ATTACKS[64];

int lsb(Bitboard bb);
int pop_lsb(Bitboard &bb);
int popcount(Bitboard bb);

void print_bitboard(Bitboard bb);
void init_attack_tables();