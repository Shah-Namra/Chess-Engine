
#include "search.h"
#include "movegen.h"
#include "eval.h"
#include "constants.h"

// recursive search
// gives the score based on current posision from white
// + white winning, - black winning

static int minimax(Board &board, int depth)
{
    if (depth == 0)
        return evaluate(board);

    MoveList moves = generate_moves(board, board.side_to_move);

    // no moves = checkmate or stalemate
    //
    if (moves.empty())
    {
        // bad for whoever is stuck,negate based on who's to move
        return (board.side_to_move == WHITE) ? -INF_SCORE + 1 : INF_SCORE - 1;
    }

    if (board.side_to_move == WHITE)
    {
        int best = -INF_SCORE;
        for (const Move &m : moves)
        {
            UndoInfo undo = board.make_move(m);
            int score = minimax(board, depth - 1);
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
            int score = minimax(board, depth - 1);
            board.unmake_move(m, undo);
            if (score < best)
                best = score;
        }
        return best;
    }
}
// need to optimize this
//

SearchResult search(Board &board, int depth)
{
    MoveList moves = generate_moves(board, board.side_to_move);

    if (moves.empty())
        return SearchResult();

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
        int score = minimax(board, depth - 1);
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

    return SearchResult(best_move, best_score);
}