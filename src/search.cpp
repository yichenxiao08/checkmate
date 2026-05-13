#include "search.h"
#include "transposition.h"
#include "searchConstants.h"
#include "evaluation.h"
#include <climits>
#include <algorithm>

Move killer_table[2][256];
int history_table[12][64]; // remember to reset

int score_move(Board &board, Move m, int ply)
{
  if (board.squares[m.to] != EMPTY)
  {

    return mvv_lva(board.squares[m.from], board.squares[m.to]);
  }
  if (board.is_same_move(m, killer_table[0][ply]))
  {
    return 90000;
  }
  if (board.is_same_move(m, killer_table[1][ply]))
  {
    return 80000;
  }
  else
    return history_table[board.squares[m.from]][m.to];
}

int mvv_lva(Piece attack, Piece victim) { return piece_vals[victim] - piece_vals[attack]; }

int negamax(Board &board, MoveGenerator &move_gen, int alpha, int beta, int depth, int ply, bool can_null)
{
  int original_alpha = alpha;
  Move best_move;
  // probe table
  int entry_score, entry_eval;
  Move entry_move;
  if (probe_entry(board.hash, depth, alpha, beta, ply, entry_score, entry_eval, entry_move))
  {
    return entry_score;
  }

  bool is_in_check = move_gen.is_in_check(board, board.is_white_to_move());
  int static_eval;
  if (is_in_check)
  {
    static_eval = NO_EVAL;
  }
  else if (entry_eval != NO_EVAL)
    static_eval = entry_eval;
  else
    static_eval = evaluate(board);

  // base case
  if (depth == 0)
    return quiescence(board, move_gen, alpha, beta, ply, static_eval);

  // null move pruning
  if (!is_in_check && depth >= 3 && beta < MATE_THRESHOLD - 256 && board.has_piece_material() && can_null)
  {
    if (static_eval >= beta)
    {
      Undo undo_null;
      board.make_null_move(undo_null);
      int score = -negamax(board, move_gen, -beta, -beta + 1, depth - (2 + depth / 6) - 1, ply + 1, false);
      board.unmake_null_move(undo_null);
      if (score >= beta)
        return score;
    }
  }

  // move ordering
  int greatest_value = -INF;
  move_gen.generate_moves(board, ply);
  int scores[218] = {0};
  for (int i = 0; i < move_gen.move_lists[ply].count; i++)
  {
    if (board.is_same_move(move_gen.move_lists[ply].moves[i], entry_move))
      scores[i] = INT_MAX;
    else
      scores[i] = score_move(board, move_gen.move_lists[ply].moves[i], ply);
  }

  // recursive call
  for (int i = 0; i < move_gen.move_lists[ply].count; i++)
  {
    int best = i;
    for (int j = i + 1; j < move_gen.move_lists[ply].count; j++)
    {
      if (scores[j] > scores[best])
      {
        best = j;
      }
    }
    std::swap(scores[i], scores[best]);
    std::swap(move_gen.move_lists[ply].moves[i], move_gen.move_lists[ply].moves[best]);

    Move m = move_gen.move_lists[ply].moves[i];
    board.make_move(m);
    if (move_gen.is_in_check(board, !board.is_white_to_move()))
    {
      board.unmake_move(m);
      continue;
    }

    bool move_is_check = move_gen.is_in_check(board, board.is_white_to_move());
    bool move_is_capture = !(m.prev_state.captured_piece == EMPTY);

    int score;

    bool reducible = (i >= LMR_MIN_INDEX) && (depth >= LMR_MIN_DEPTH) && !move_is_check && !is_in_check && !move_is_capture && m.promotion_piece == EMPTY && !board.is_same_move(m, killer_table[0][ply]) && !board.is_same_move(m, killer_table[1][ply]);

    if (reducible)
    {
      int R = 1;
      score = -negamax(board, move_gen, -alpha - 1, -alpha, depth - 1 - R, ply + 1, true);
      if (score > alpha)
      {
        score = -negamax(board, move_gen, -beta, -alpha, depth - 1, ply + 1, true);
      }
    }
    else
    {
      score = -negamax(board, move_gen, -beta, -alpha, depth - 1, ply + 1, true);
    }

    if (score > greatest_value)
    {
      greatest_value = score;
      best_move = m;
      if (score > alpha)
      {
        alpha = score;
      }
    }
    if (score >= beta)
    {
      if (!move_is_capture)
      {
        killer_table[1][ply] = killer_table[0][ply];
        killer_table[0][ply] = m;
        history_table[board.squares[m.to]][m.to] += depth * depth;
      }
      board.unmake_move(m);
      break;
    }
    board.unmake_move(m);
  }

  // game ending situation check
  if (greatest_value == -INF)
  {
    if (is_in_check)
    {
      return -MATE_THRESHOLD + ply;
    }
    else
      return 0;
  }

  // store table
  int node_type;
  if (greatest_value <= original_alpha)
    node_type = UPPER;
  else if (greatest_value >= beta)
    node_type = LOWER;
  else
    node_type = EXACT;
  store_entry(board.hash, greatest_value, depth, best_move, node_type, ply, static_eval);
  return greatest_value;
}

Move root_negamax(Board &board, MoveGenerator &move_gen, int depth)
{
  Move best_move;
  int best_score = -INF;

  move_gen.generate_moves(board, 0);
  for (int i = 0; i < move_gen.move_lists[0].count; i++)
  {
    Move m = move_gen.move_lists[0].moves[i];
    board.make_move(m);
    int score = -negamax(board, move_gen, -INF, INF, depth - 1, 1, true);
    board.unmake_move(m);

    if (score > best_score)
    {
      best_score = score;
      best_move = m;
    }
  }
  return best_move;
}

int quiescence(Board &board, MoveGenerator &move_gen, int alpha, int beta, int ply, int static_eval)
{
  bool is_in_check = move_gen.is_in_check(board, board.is_white_to_move());
  int greatest_val = -INF;
  if (!is_in_check)
  {
    int stand_pat = (static_eval != NO_EVAL) ? static_eval : evaluate(board);
    if (stand_pat > greatest_val)
      greatest_val = stand_pat;
    if (stand_pat > alpha)
      alpha = stand_pat;
    if (stand_pat >= beta)
      return stand_pat;
  }

  if (is_in_check)
  {
    move_gen.generate_moves(board, ply);
    for (int i = 0; i < move_gen.move_lists[ply].count; i++)
    {
      Move m = move_gen.move_lists[ply].moves[i];
      board.make_move(m);
      if (move_gen.is_in_check(board, !board.is_white_to_move()))
      {
        board.unmake_move(m);
        continue;
      }
      int score = -quiescence(board, move_gen, -beta, -alpha, ply + 1, NO_EVAL);
      board.unmake_move(m);
      if (score >= beta)
        return score;
      if (score > greatest_val)
        greatest_val = score;
      if (score > alpha)
        alpha = score;
    }
  }
  else
  {
    move_gen.generate_captures(board, ply);
    for (int i = 0; i < move_gen.move_lists[ply].count; i++)
    {
      Move m = move_gen.move_lists[ply].moves[i];
      board.make_move(m);
      if (move_gen.is_in_check(board, !board.is_white_to_move()))
      {
        board.unmake_move(m);
        continue;
      }
      int score = -quiescence(board, move_gen, -beta, -alpha, ply + 1, NO_EVAL);
      board.unmake_move(m);
      if (score >= beta)
        return score;
      if (score > greatest_val)
        greatest_val = score;
      if (score > alpha)
        alpha = score;
    }
  }
  if (greatest_val == -INF && is_in_check)
    return -MATE_THRESHOLD + ply;
  return greatest_val;
}
