#include "movesort.h"
#include "constants.h"

#include <algorithm>
#include <cctype>

static int char_value(char p)
{
    switch (std::toupper(p))
    {
    case 'P':
        return PAWN_VALUE;
    case 'N':
        return KNIGHT_VALUE;
    case 'B':
        return BISHOP_VALUE;
    case 'R':
        return ROOK_VALUE;
    case 'Q':
        return QUEEN_VALUE;
    case 'K':
        return KING_VALUE;

    default:
        return 0;
    }
}

static inline int sq_to_row(int sq)
{
    return 7 - (sq / 8);
}

static inline int sq_to_col(int sq)
{
    return sq % 8;
}

int mvv_lva_score(char attacker, char victim)
{
    // Most Valuable Victim Least Valuable Attacker
    return char_value(victim) * 10 - char_value(attacker);
}

void order_moves(MoveList &moves, const Board &board, Move tt_move)
{
    // Store moves with scores for sorting
    struct ScoredMove
    {
        Move move;
        int score;
    };

    std::vector<ScoredMove> scored;
    scored.reserve(moves.size());

    for (const Move &m : moves)
    {
        int score = 0;

        // TT move gets highest priority
        if (m.from_sq == tt_move.from_sq &&
            m.to_sq == tt_move.to_sq &&
            m.flags == tt_move.flags)
        {
            score = 1000000;
        }

        else if (m.flags == FLAG_CAPTURE ||
                 m.flags == FLAG_PROMO_QUEEN)
        {
            char attacker =
                board.get_piece(
                    sq_to_row(m.from_sq),
                    sq_to_col(m.from_sq));

            char victim =
                board.get_piece(
                    sq_to_row(m.to_sq),
                    sq_to_col(m.to_sq));

            score = 100000 +
                    mvv_lva_score(attacker, victim);
        }

        // Quiet moves keep score 0
        scored.push_back({m, score});
    }

    // Sort highest score first
    std::stable_sort(
        scored.begin(),
        scored.end(),
        [](const ScoredMove &a, const ScoredMove &b)
        {
            return a.score > b.score;
        });

    // Copy sorted moves back
    for (int i = 0; i < (int)moves.size(); i++)
    {
        moves[i] = scored[i].move;
    }
}