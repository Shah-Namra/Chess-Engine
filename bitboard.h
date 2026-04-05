#pragma once

// bitboard.h
// Bitboard types and helpers.
// Each bit = square (0=A1 ... 63=H8).

#include <cstdint>
#include <iostream>

// 64-bit board.
using Bitboard = uint64_t;

// File masks (prevent wrap on horizontal shifts).
const Bitboard NOT_A_FILE = 0xFEFEFEFEFEFEFEFEULL;
const Bitboard NOT_H_FILE = 0x7F7F7F7F7F7F7F7FULL;

// Rank masks.
const Bitboard RANK_2 = 0x000000000000FF00ULL;
const Bitboard RANK_7 = 0x00FF000000000000ULL;

// Shifts (north/south safe, east/west masked).
inline Bitboard shift_north(Bitboard b) { return b << 8; }
inline Bitboard shift_south(Bitboard b) { return b >> 8; }
inline Bitboard shift_east(Bitboard b) { return (b & NOT_H_FILE) << 1; }
inline Bitboard shift_west(Bitboard b) { return (b & NOT_A_FILE) >> 1; }
inline Bitboard shift_ne(Bitboard b) { return (b & NOT_H_FILE) << 9; }
inline Bitboard shift_nw(Bitboard b) { return (b & NOT_A_FILE) << 7; }
inline Bitboard shift_se(Bitboard b) { return (b & NOT_H_FILE) >> 7; }
inline Bitboard shift_sw(Bitboard b) { return (b & NOT_A_FILE) >> 9; }

// Attack tables (init at startup).
extern Bitboard KNIGHT_ATTACKS[64];
extern Bitboard KING_ATTACKS[64];

// Bit helpers.
int lsb(Bitboard bb);
int pop_lsb(Bitboard &bb);
int popcount(Bitboard bb);

void print_bitboard(Bitboard bb);
void init_attack_tables();