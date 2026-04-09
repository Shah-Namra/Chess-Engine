#pragma once

// constants.h
// Engine-wide constants (values in centipawns).

// --- Piece values ---
const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;
const int KING_VALUE = 20000;

// --- Move flags ---
const int FLAG_QUIET = 0;
const int FLAG_CAPTURE = 1;
const int FLAG_DOUBLE_PUSH = 2;
const int FLAG_EN_PASSANT = 3;
const int FLAG_PROMO_QUEEN = 4;
const int FLAG_PROMO_ROOK = 5;
const int FLAG_PROMO_BISHOP = 6;
const int FLAG_PROMO_KNIGHT = 7;

// --- Search limits ---
const int MAX_DEPTH = 64;
const int INF_SCORE = 99999;