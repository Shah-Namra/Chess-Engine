#pragma once

// types.h
// these enums aren't used yet but we're going to need them everywhere once
// we switch to bitboards in Phase 3. putting them here now so there's a
// clear place for shared types — otherwise they end up scattered across files
// and then nothing includes the right thing and it's a mess

enum Color
{
    WHITE,
    BLACK
};

enum PieceType
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE // empty square
};
