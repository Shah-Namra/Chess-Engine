#include "search.h"
#include "movegen.h"
#include "eval.h"
#include "hash.h"
#include "tt.h"
#include "movesort.h"
#include "constants.h"

int nodes_searched = 0;
// added node searched to compare both methods
// ideally aplha ebta should have low nodes

static int minimax_impl(Board &board, int depth)
{
    nodes_searched++;
    if (depth == 0)
        return evaluate(board);

    MoveList moves = generate_legal_moves(board, board.side_to_move);
    
    // no moves = checkmate or stalemate
    //
    if (moves.empty()) {
        // checkmate or stalemate — in_check() tells us which
        if (in_check(board, board.side_to_move))
            return (board.side_to_move == WHITE) ? -INF_SCORE + 1 : INF_SCORE - 1;
        return 0; // stalemate
    }

    if (board.side_to_move == WHITE)
    {
        int best = -INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = minimax_impl(board, depth - 1);
            board.unmake_move(m, undo);
            if (score > best)
                best = score;
        }
        return best;
    }
    else
    {
        int best = INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = minimax_impl(board, depth - 1);
            board.unmake_move(m, undo);
            if (score < best)
                best = score;
        }
        return best;
    }
}

SearchResult search_minimax(Board &board, int depth)
{
    MoveList moves = generate_legal_moves(board, board.side_to_move);
    if (moves.empty())
        return SearchResult();

    nodes_searched = 0;
    // best move and score for the curr postion
    // first move might be bad, but something will be return if everything is bad
    // ToDO: handle no moves = checkmate/stalemate
    // optimize by initializing best move and score to the first move, then comparing against it
    // ToDo: for black we need to minimize score, for white max score
    Move best_move = moves[0];
    int best_score = (board.side_to_move == WHITE) ? -INF_SCORE : INF_SCORE;

    for (const Move &m : moves)
    {
        UndoInfo undo = board.make_move(m);
        int score = minimax_impl(board, depth - 1);
        board.unmake_move(m, undo);

        if (board.side_to_move == WHITE && score > best_score)
        {
            best_score = score;
            best_move = m;
        }
        if (board.side_to_move == BLACK && score < best_score)
        {
            best_score = score;
            best_move = m;
        }
    }

    return SearchResult(best_move, best_score, nodes_searched);
}
// tried alpha beta
//  alpha is for white, beta for black
//  stop node when white move >= beta, black is winning in this move so stop
//  same for black <= alpha, white is winning
//  update alpha beta when find better move for white or black

static int alphabeta(Board &board, int depth, int alpha, int beta)
{
    nodes_searched++;

    if (depth == 0)
        return evaluate(board);

    MoveList moves = generate_legal_moves(board, board.side_to_move);
    
    if (moves.empty())
    {
        // checkmate or stalemate: in_check() tells us which
        if (in_check(board, board.side_to_move))
            return (board.side_to_move == WHITE) ? -INF_SCORE + 1 : INF_SCORE - 1;
        return 0; // stalemate
    }

    if (board.side_to_move == WHITE)
    {
        int best = -INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = alphabeta(board, depth - 1, alpha, beta);
            board.unmake_move(m, undo);

            if (score > best)
                best = score;
            if (best > alpha)
                alpha = best;

                // beta should stop here if the white has a better move then black
            if (alpha >= beta)
                break;
        }
        return best;
    }
    else
    {
        int best = INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = alphabeta(board, depth - 1, alpha, beta);
            board.unmake_move(m, undo);

            if (score < best)
                best = score;
            if (best < beta)
                beta = best;

            // alpha cutoff: symmetric to the beta cutoff above.
            if (alpha >= beta)
                break;
        }
        return best;
    }
}

SearchResult search_alphabeta(Board &board, int depth)
{
    MoveList moves = generate_legal_moves(board, board.side_to_move);
    if (moves.empty())
        return SearchResult();

    nodes_searched = 0;

    Move best_move = moves[0];
    int best_score = (board.side_to_move == WHITE) ? -INF_SCORE : INF_SCORE;

    // initial window: full range
    int alpha = -INF_SCORE;
    int beta = INF_SCORE;

    for (const Move &m : moves)
    {
        UndoInfo undo = board.make_move(m);
        int score = alphabeta(board, depth - 1, alpha, beta);
        board.unmake_move(m, undo);

        if (board.side_to_move == WHITE && score > best_score)
        {
            best_score = score;
            best_move = m;
            if (best_score > alpha)
                alpha = best_score;
        }
        if (board.side_to_move == BLACK && score < best_score)
        {
            best_score = score;
            best_move = m;
            if (best_score < beta)
                beta = best_score;
        }
    }

    return SearchResult(best_move, best_score, nodes_searched);
}
// Alpha beta + Transposition Table

static int alphabeta_tt(Board &board, int depth, int alpha, int beta)
{
    nodes_searched++;

    // check TT before searching
    TTEntry *entry = TT.probe(board.zobrist_hash);

    // reuse cached result if depth is sufficient
    if (entry && entry->depth >= depth)
    {
        if (entry->flag == TT_EXACT)
            return entry->score;
        if (entry->flag == TT_ALPHA && entry->score <= alpha)
            return alpha;
        if (entry->flag == TT_BETA && entry->score >= beta)
            return beta;
    }
    // leaf node
    if (depth == 0)
    {
        int score = evaluate(board);
        TT.store(board.zobrist_hash, 0, score, TT_EXACT, Move(0, 0, 0));
        return score;
    }

    MoveList moves = generate_legal_moves(board, board.side_to_move);
    if (moves.empty())
     {
        if (in_check(board, board.side_to_move))
            return (board.side_to_move == WHITE) ? -INF_SCORE + 1 : INF_SCORE - 1;
        return 0; // stalemate
    }

    int original_alpha = alpha;
    Move best_move = moves[0];

    if (board.side_to_move == WHITE)
    {
        int best = -INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = alphabeta_tt(board, depth - 1, alpha, beta);
            board.unmake_move(m, undo);
            if (score > best)
            {
                best = score;
                best_move = m;
            }
            if (best > alpha)
                alpha = best;
            if (alpha >= beta)
                break; // beta cutoff
        }
        // store result with correct flag
        int flag = (best <= original_alpha) ? TT_ALPHA
                   : (best >= beta)         ? TT_BETA
                                            : TT_EXACT;
        TT.store(board.zobrist_hash, depth, best, flag, best_move);
        return best;
    }
    else
    {
        int best = INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = alphabeta_tt(board, depth - 1, alpha, beta);
            board.unmake_move(m, undo);
            if (score < best)
            {
                best = score;
                best_move = m;
            }
            if (best < beta)
                beta = best;
            if (alpha >= beta)
                break; // alpha cutoff
        }
        int flag = (best >= beta)             ? TT_BETA
                   : (best <= original_alpha) ? TT_ALPHA
                                              : TT_EXACT;
        TT.store(board.zobrist_hash, depth, best, flag, best_move);
        return best;
    }
}

SearchResult search_tt(Board &board, int depth)
{
    MoveList moves = generate_legal_moves(board, board.side_to_move);
    if (moves.empty())
        return SearchResult();

    TT.clear();
    nodes_searched = 0;

    Move best_move = moves[0];
    int best_score = (board.side_to_move == WHITE) ? -INF_SCORE : INF_SCORE;
    int alpha = -INF_SCORE, beta = INF_SCORE;

    for (const Move &m : moves)
    {
        UndoInfo undo = board.make_move(m);
        int score = alphabeta_tt(board, depth - 1, alpha, beta);
        board.unmake_move(m, undo);
        if (board.side_to_move == WHITE && score > best_score)
        {
            best_score = score;
            best_move = m;
            if (best_score > alpha)
                alpha = best_score;
        }
        if (board.side_to_move == BLACK && score < best_score)
        {
            best_score = score;
            best_move = m;
            if (best_score < beta)
                beta = best_score;
        }
    }
    return SearchResult(best_move, best_score, nodes_searched);
}

// Alpha-beta + TT + Move ordering
static int alphabeta_ordered(Board &board, int depth, int alpha, int beta)
{
    nodes_searched++;

    TTEntry *entry = TT.probe(board.zobrist_hash);
    Move tt_move(0, 0, 0);

    if (entry && entry->depth >= depth)
    {
        if (entry->flag == TT_EXACT)
            return entry->score;
        if (entry->flag == TT_ALPHA && entry->score <= alpha)
            return alpha;
        if (entry->flag == TT_BETA && entry->score >= beta)
            return beta;
    }
    if (entry)
        tt_move = entry->best_move;

    if (depth == 0)
    {
        int score = evaluate(board);
        TT.store(board.zobrist_hash, 0, score, TT_EXACT, Move(0, 0, 0));
        return score;
    }

    MoveList moves = generate_legal_moves(board, board.side_to_move);
    if (moves.empty())
    {
        if (in_check(board, board.side_to_move))
            return (board.side_to_move == WHITE) ? -INF_SCORE + 1 : INF_SCORE - 1;
        return 0;// stalemate
    }

    order_moves(moves, board, tt_move, 0);

    int original_alpha = alpha;
    Move best_move = moves[0];

    if (board.side_to_move == WHITE)
    {
        int best = -INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = alphabeta_ordered(board, depth - 1, alpha, beta);
            board.unmake_move(m, undo);
            if (score > best)
            {
                best = score;
                best_move = m;
            }
            if (best > alpha)
                alpha = best;
            if (alpha >= beta)
                break;
        }
        int flag = (best <= original_alpha) ? TT_ALPHA
                   : (best >= beta)         ? TT_BETA
                                            : TT_EXACT;
        TT.store(board.zobrist_hash, depth, best, flag, best_move);
        return best;
    }
    else
    {
        int best = INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = alphabeta_ordered(board, depth - 1, alpha, beta);
            board.unmake_move(m, undo);
            if (score < best)
            {
                best = score;
                best_move = m;
            }
            if (best < beta)
                beta = best;
            if (alpha >= beta)
                break;
        }
        int flag = (best >= beta)             ? TT_BETA
                   : (best <= original_alpha) ? TT_ALPHA
                                              : TT_EXACT;
        TT.store(board.zobrist_hash, depth, best, flag, best_move);
        return best;
    }
}

SearchResult search_ordered(Board &board, int depth)
{
    MoveList moves = generate_legal_moves(board, board.side_to_move);
    if (moves.empty())
        return SearchResult();

    TT.clear();
    nodes_searched = 0;

    TTEntry *entry = TT.probe(board.zobrist_hash);
    Move tt_move(0, 0, 0);
    if (entry)
        tt_move = entry->best_move;

    order_moves(moves, board, tt_move, 0);

    Move best_move = moves[0];
    int best_score = (board.side_to_move == WHITE) ? -INF_SCORE : INF_SCORE;
    int alpha = -INF_SCORE, beta = INF_SCORE;

    for (const Move &m : moves)
    {
        UndoInfo undo = board.make_move(m);
        int score = alphabeta_ordered(board, depth - 1, alpha, beta);
        board.unmake_move(m, undo);
        if (board.side_to_move == WHITE && score > best_score)
        {
            best_score = score;
            best_move = m;
            if (best_score > alpha)
                alpha = best_score;
        }
        if (board.side_to_move == BLACK && score < best_score)
        {
            best_score = score;
            best_move = m;
            if (best_score < beta)
                beta = best_score;
        }
    }
    return SearchResult(best_move, best_score, nodes_searched);
}
// Quiescence Search
// normal evaluation at depth 0 can miss tactical sequences.
// example:
//   white captures queen
//   engine stops search and thinks white is winning
//   but black immediately recaptures next move
//
// quiescence search fixes this by extending only "noisy" moves
// (captures/promotions) until the position becomes stable.

static int quiesce(Board &board, int alpha, int beta)
{
    nodes_searched++;

    // static evaluation before searching captures
    int stand_pat = evaluate(board);

    // stand-pat pruning
    // side to move can always choose to stop capturing
    if (board.side_to_move == WHITE)
    {
        if (stand_pat >= beta)
            return beta;

            if (stand_pat > alpha)
            alpha = stand_pat;
    }
    else
    {
        if (stand_pat <= alpha)
            return alpha;

            if (stand_pat < beta)
            beta = stand_pat;
    }
    // generate legal moves
    MoveList all_moves = generate_legal_moves(board, board.side_to_move);

    // keep only captures/promotions
    MoveList captures;
    for (const Move &m : all_moves)
    {
        if (m.flags == FLAG_CAPTURE || m.flags == FLAG_PROMO_QUEEN)
            captures.push_back(m);
    }

    // quiet position reached
    if (captures.empty())
        return stand_pat;

    // order captures for faster cutoffs
    order_moves(captures, board, Move(0, 0, 0), 0);

    // maximizing side
    if (board.side_to_move == WHITE)
    {
        int best = stand_pat;
        for (const Move &m : captures)
        {
            UndoInfo undo = board.make_move(m);
            int score = quiesce(board, alpha, beta);
            board.unmake_move(m, undo);
            if (score > best)
                best = score;
            if (best > alpha)
                alpha = best;
            // beta cutoff
            if (alpha >= beta)
                break;
        }
        return best;
    }
    // minimizing side
    else
    {
        int best = stand_pat;
        for (const Move &m : captures)
        {
            UndoInfo undo = board.make_move(m);
            int score = quiesce(board, alpha, beta);
            board.unmake_move(m, undo);
            if (score < best)
                best = score;
            if (best < beta)
                beta = best;
            if (alpha >= beta)
                break;
        }
        return best;
    }
}

// alpha-beta with quiescence at leaf nodes
static int alphabeta_q(Board &board, int depth, int alpha, int beta, int ply)
{
    nodes_searched++;

        // transposition table lookup
    TTEntry *entry = TT.probe(board.zobrist_hash);
    Move tt_move(0, 0, 0);

    if (entry && entry->depth >= depth)
    {
        if (entry->flag == TT_EXACT)
            return entry->score;
        if (entry->flag == TT_ALPHA && entry->score <= alpha)
            return alpha;
        if (entry->flag == TT_BETA && entry->score >= beta)
            return beta;
    }
    // save TT move for move ordering
    if (entry)
        tt_move = entry->best_move;

    // use quiescence instead of evaluate()
    if (depth == 0)
        return quiesce(board, alpha, beta);

    MoveList moves = generate_legal_moves(board, board.side_to_move);

    // checkmate or stalemate
    if (moves.empty())
    {
        if (in_check(board, board.side_to_move))
            return (board.side_to_move == WHITE) ? -INF_SCORE + 1 : INF_SCORE - 1;
        return 0; // stalemate
    }

    // move ordering improves pruning
     order_moves(moves, board, tt_move, ply);

    int original_alpha = alpha;
    
    Move best_move = moves[0];
    int color = (board.side_to_move == WHITE) ? 0 : 1;

    // maximizing player
    if (board.side_to_move == WHITE)
    {
        int best = -INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = alphabeta_q(board, depth - 1, alpha, beta, ply + 1);
            board.unmake_move(m, undo);

            if (score > best)
            {
                best = score;
                best_move = m;
            }
            if (best > alpha)
                alpha = best;

            if (alpha >= beta)
            {
                // killer + history update only for quiet moves
                bool is_quiet = (m.flags != FLAG_CAPTURE &&
                                 m.flags != FLAG_PROMO_QUEEN);
                if (is_quiet)
                {
                    update_killers(m, ply);
                    update_history(m, color, depth);
                }
                break;
            }
        }
        int flag = (best <= original_alpha) ? TT_ALPHA
                   : (best >= beta)         ? TT_BETA
                                            : TT_EXACT;
        TT.store(board.zobrist_hash, depth, best, flag, best_move);
        return best;
    }
    else
    {
        int best = INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = alphabeta_q(board, depth - 1, alpha, beta, ply + 1);
            board.unmake_move(m, undo);

            if (score < best)
            {
                best = score;
                best_move = m;
            }
            if (best < beta)
                beta = best;

            if (alpha >= beta)
            {
                bool is_quiet = (m.flags != FLAG_CAPTURE &&
                                 m.flags != FLAG_PROMO_QUEEN);
                if (is_quiet)
                {
                    update_killers(m, ply);
                    update_history(m, color, depth);
                }
                break;
            }
        }
        int flag = (best >= beta)             ? TT_BETA
                   : (best <= original_alpha) ? TT_ALPHA
                                              : TT_EXACT;
        TT.store(board.zobrist_hash, depth, best, flag, best_move);
        return best;
    }
}

SearchResult search_quiescence(Board &board, int depth)
{
    MoveList moves = generate_legal_moves(board, board.side_to_move);
    if (moves.empty())
        return SearchResult();

    nodes_searched = 0;

    // try TT move first if available (from previous ID iteration)
    TTEntry *entry = TT.probe(board.zobrist_hash);
    Move tt_move(0, 0, 0);
    if (entry)
        tt_move = entry->best_move;

    order_moves(moves, board, tt_move, 0);

    Move best_move = moves[0];
    int best_score = (board.side_to_move == WHITE) ? -INF_SCORE : INF_SCORE;
    int alpha = -INF_SCORE;
    int beta = INF_SCORE;

    for (const Move &m : moves)
    {
        UndoInfo undo = board.make_move(m);
        int score = alphabeta_q(board, depth - 1, alpha, beta, 1);
        board.unmake_move(m, undo);

        if (board.side_to_move == WHITE && score > best_score)
        {
            best_score = score;
            best_move = m;
            if (best_score > alpha)
                alpha = best_score;
        }
     
        if (board.side_to_move == BLACK && score < best_score)
        {
            best_score = score;
            best_move = m;
            
            if (best_score < beta)
                beta = best_score;
        }
    }

    return SearchResult(best_move, best_score, nodes_searched);
}

// iterative deepening
    // search depth 1, 2, 3... up to max_depth
    // each pass fills the TT which orders the next pass better
    // this makes deeper searches much faster than going straight to max_depth.
SearchResult search_iterative(Board &board, int max_depth)
{
    TT.clear();
    clear_search_tables();

    SearchResult best;
    for (int depth = 1; depth <= max_depth; depth++)
    {
        //TT.clear();
        best = search_quiescence(board, depth);
    }
    return best;
}