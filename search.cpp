
#include "search.h"
#include "movegen.h"
#include "eval.h"
#include "constants.h"

int nodes_searched = 0;
// added node searched to compare both methods
// ideally aplha ebta should have low nodes

static int minimax_impl(Board &board, int depth)
{
    nodes_searched++;
    if (depth == 0)
        return evaluate(board);

    MoveList moves = generate_moves(board, board.side_to_move);

    // no moves = checkmate or stalemate
    //
    if (moves.empty())
        // bad for whoever is stuck,negate based on who's to move
        return (board.side_to_move == WHITE) ? -INF_SCORE + 1 : INF_SCORE - 1;

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
    MoveList moves = generate_moves(board, board.side_to_move);
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

    MoveList moves = generate_moves(board, board.side_to_move);

    if (moves.empty())
        return (board.side_to_move == WHITE) ? -INF_SCORE + 1 : INF_SCORE - 1;

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
    MoveList moves = generate_moves(board, board.side_to_move);
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