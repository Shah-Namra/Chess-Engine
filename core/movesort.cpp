#include "movesort.h"
#include "constants.h"

#include <algorithm>
#include <cctype>
#include <cstring>

// killer + history storage
Move killers[MAX_PLY][2] = {};
int history[2][64][64] = {};

void clear_search_tables()
{
    for (int p = 0; p < MAX_PLY; p++)
    {
        killers[p][0] = Move(0, 0, 0);
        killers[p][1] = Move(0, 0, 0);
    }
    std::memset(history, 0, sizeof(history));
}

// two moves are "the same move" if from/to/flags match
static inline bool same_move(const Move &a, const Move &b)
{
    return a.from_sq == b.from_sq &&
           a.to_sq   == b.to_sq   &&
           a.flags   == b.flags;
}

void update_killers(const Move &m, int ply)
{
    if (ply < 0 || ply >= MAX_PLY)
        return;

    // dont store duplicate in both slots
    if (same_move(killers[ply][0], m))
        return;

    // shift: old killer[0] becomes killer[1], new move becomes killer[0]
    killers[ply][1] = killers[ply][0];
    killers[ply][0] = m;
}

void update_history(const Move &m, int color, int depth)
{
    // bonus is depth*depth: deeper cutoffs are worth more
    // cap to avoid overflow on long searches
    int bonus = depth * depth;
    int &h = history[color][m.from_sq][m.to_sq];
    h += bonus;
    if (h > 1000000)
        h = 1000000;
}

static int char_value(char p)
{
    switch (std::toupper(p))
    {
    case 'P': return PAWN_VALUE;
    case 'N': return KNIGHT_VALUE;
    case 'B': return BISHOP_VALUE;
    case 'R': return ROOK_VALUE;
    case 'Q': return QUEEN_VALUE;
    case 'K': return KING_VALUE;
    default:  return 0;
    }
}

static inline int sq_to_row(int sq) { return 7 - (sq / 8); }
static inline int sq_to_col(int sq) { return sq % 8; }

int mvv_lva_score(char attacker, char victim)
{
    return char_value(victim) * 10 - char_value(attacker);
}

void order_moves(MoveList &moves, const Board &board, Move tt_move, int ply)
{
    struct ScoredMove
    {
        Move move;
        int score;
    };

    std::vector<ScoredMove> scored;
    scored.reserve(moves.size());

    int color = (board.side_to_move == WHITE) ? 0 : 1;
    bool have_killers = (ply >= 0 && ply < MAX_PLY);

    // a tt_move with from==to is treated as "no tt move"
    bool tt_valid = (tt_move.from_sq != tt_move.to_sq);

    for (const Move &m : moves)
    {
        int score = 0;

        // TT move gets highest priority
        if (tt_valid && same_move(m, tt_move))
        {
            score = 1000000;
        }
        // 2. captures + queen promotions — MVV-LVA
        else if (m.flags == FLAG_CAPTURE || m.flags == FLAG_PROMO_QUEEN || m.flags == FLAG_EN_PASSANT)
        {
            char attacker = board.get_piece(sq_to_row(m.from_sq), sq_to_col(m.from_sq));
            char victim   = board.get_piece(sq_to_row(m.to_sq),   sq_to_col(m.to_sq));
            score = 100000 + mvv_lva_score(attacker, victim);
        }
        // 3. killer moves: quiet moves that caused cutoffs at this ply before
        else if (have_killers && same_move(m, killers[ply][0]))
        {
            score = 90000;
        }
        else if (have_killers && same_move(m, killers[ply][1]))
        {
            score = 80000;
        }
        // 4. history heuristic: broader quiet move ordering
        else
        {
            score = history[color][m.from_sq][m.to_sq];
        }

        scored.push_back({m, score});
    }

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