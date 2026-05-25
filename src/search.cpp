#include "search.h"
#include "transposition.h"
#include "searchConstants.h"
#include "evaluation.h"
#include <climits>
#include <atomic>
#include <algorithm>

Move killer_table[2][256];
int history_table[12][64];
u64 repetition_table[1024];
int repetition_count = 0;

void reset_killer_table()
{
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 256; j++)
      killer_table[i][j] = Move();
}

void reset_history_table()
{
  for (int i = 0; i < 12; i++)
    for (int j = 0; j < 64; j++)
      history_table[i][j] = 0;
}

void reset_tt()
{
  for (int i = 0; i < table_size; i++)
    table[i] = HashEntry{};
}

int score_move(Board &board, Move m, int ply)
{
  if (board.squares[m.to] != EMPTY)
  {
    return mvv_lva(board.squares[m.from], board.squares[m.to]);
  }
  if (board.is_same_move(m, killer_table[0][ply]))
    return 90000;
  if (board.is_same_move(m, killer_table[1][ply]))
    return 80000;
  return history_table[board.squares[m.from]][m.to];
}

int mvv_lva(Piece attack, Piece victim) { return (piece_vals[victim] - piece_vals[attack]) * 100000; }

int negamax(Board &board, MoveGenerator &move_gen, int alpha, int beta, int depth, int ply, bool can_null, std::atomic<bool> &stop_flag)
{
  if (board.half_move_count >= 100)
    return 0;
  if (ply > 0 && is_repetition(board))
    return 0;

  int original_alpha = alpha;
  Move best_move;
  int entry_score = 0;
  int entry_eval = NO_EVAL;
  Move entry_move;
  bool tt_hit = probe_entry(board.hash, depth, alpha, beta, ply, entry_score, entry_eval, entry_move);
  if (tt_hit)
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
    static_eval = evaluate_position(board, move_gen);

  // base case
  if (depth == 0)
  {
    return quiescence(board, move_gen, alpha, beta, ply, static_eval, 0, stop_flag);
  }

  if (stop_flag.load(std::memory_order_relaxed))
    return 0;

  // static null move pruning
  if (!is_in_check && depth <= 8)
  {
    int margin = 80 * depth;
    if (static_eval >= beta + margin)
    {
      return static_eval;
    }
  }

  // null move pruning
  if (!is_in_check && depth >= 3 && (beta < MATE_THRESHOLD - 256 || beta > MATE_THRESHOLD) && board.has_piece_material() && can_null)
  {
    if (static_eval >= beta)
    {
      Undo undo_null;
      board.make_null_move(undo_null);
      int score = -negamax(board, move_gen, -beta, -beta + 1, depth - (2 + depth / 6) - 1, ply + 1, false, stop_flag);
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
  Move quiets_searched[218];
  Piece quiets_pieces[218];
  int quiets_count = 0;
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
    Piece piece_moved = board.squares[m.from];
    board.make_move(m);

    if (move_gen.is_in_check(board, !board.is_white_to_move()))
    {
      board.unmake_move(m);
      continue;
    }

    bool move_is_check = move_gen.is_in_check(board, board.is_white_to_move());
    bool move_is_capture = !(m.prev_state.captured_piece == EMPTY);

    int score;

    bool is_killer = board.is_same_move(m, killer_table[0][ply]) || board.is_same_move(m, killer_table[1][ply]);
    bool reducible = (i >= LMR_MIN_INDEX) && (depth >= LMR_MIN_DEPTH) && !move_is_check && !is_in_check && !move_is_capture && m.promotion_piece == EMPTY && !is_killer;

    if (i == 0)
    {
      score = -negamax(board, move_gen, -beta, -alpha, depth - 1, ply + 1, true, stop_flag);
    }
    else
    {
      if (reducible)
      {
        int R = 1 + (depth / 3) + (i / 6);
        R = std::min(R, depth - 1);
        score = -negamax(board, move_gen, -alpha - 1, -alpha, depth - 1 - R, ply + 1, true, stop_flag);
        if (score > alpha)
        {
          score = -negamax(board, move_gen, -alpha - 1, -alpha, depth - 1, ply + 1, true, stop_flag);
          if (score > alpha)
          {
            score = -negamax(board, move_gen, -beta, -alpha, depth - 1, ply + 1, true, stop_flag);
          }
        }
      }
      else
      {
        score = -negamax(board, move_gen, -alpha - 1, -alpha, depth - 1, ply + 1, true, stop_flag);
        if (score > alpha)
        {
          score = -negamax(board, move_gen, -beta, -alpha, depth - 1, ply + 1, true, stop_flag);
        }
      }
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
      if (!move_is_capture && m.promotion_piece == EMPTY)
      {
        killer_table[1][ply] = killer_table[0][ply];
        killer_table[0][ply] = m;
        int bonus = 300 * depth - 250;
        int &hm = history_table[piece_moved][m.to];
        hm += bonus;
        hm = std::clamp(hm, -30000, 30000);
        for (int q = 0; q < quiets_count; q++)
        {
          int &h = history_table[quiets_pieces[q]][quiets_searched[q].to];
          h -= bonus;
          h = std::clamp(h, -30000, 30000);
        }
      }
      board.unmake_move(m);
      break;
    }
    if (!move_is_capture && m.promotion_piece == EMPTY)
    {
      quiets_searched[quiets_count] = m;
      quiets_pieces[quiets_count] = piece_moved;
      quiets_count++;
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

Move root_negamax(Board &board, MoveGenerator &move_gen, int depth, std::atomic<bool> &stop_flag)
{
  Move best_move;
  int best_score = -INF;

  move_gen.generate_moves(board, 0);
  for (int i = 0; i < move_gen.move_lists[0].count; i++)
  {
    if (best_score > -INF && stop_flag.load(std::memory_order_relaxed))
      break;
    Move m = move_gen.move_lists[0].moves[i];
    board.make_move(m);
    if (move_gen.is_in_check(board, !board.is_white_to_move()))
    {
      board.unmake_move(m);
      continue;
    }
    int score = -negamax(board, move_gen, -INF, INF, depth - 1, 1, true, stop_flag);
    board.unmake_move(m);

    if (score > best_score)
    {
      best_score = score;
      best_move = m;
    }
  }
  return best_move;
}

int quiescence(Board &board, MoveGenerator &move_gen, int alpha, int beta, int ply, int static_eval, int depth, std::atomic<bool> &stop_flag)
{
  if (stop_flag.load(std::memory_order_relaxed))
    return 0;
  if (depth >= 8)
  {
    return (static_eval != NO_EVAL) ? static_eval : evaluate_position(board, move_gen);
  }

  bool is_in_check = move_gen.is_in_check(board, board.is_white_to_move());
  int greatest_val = -INF;

  if (is_in_check)
  {
    int scores[218] = {0};
    move_gen.generate_moves(board, ply);

    for (int i = 0; i < move_gen.move_lists[ply].count; i++)
    {
      scores[i] = score_move(board, move_gen.move_lists[ply].moves[i], ply);
    }

    for (int i = 0; i < move_gen.move_lists[ply].count; i++)
    {
      int best = i;
      for (int j = i + 1; j < move_gen.move_lists[ply].count; j++)
      {
        if (scores[j] > scores[best])
          best = j;
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
      int score = -quiescence(board, move_gen, -beta, -alpha, ply + 1, NO_EVAL, depth + 1, stop_flag);
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
    int stand_pat = (static_eval != NO_EVAL) ? static_eval : evaluate_position(board, move_gen);
    if (stand_pat > greatest_val)
      greatest_val = stand_pat;
    if (stand_pat > alpha)
      alpha = stand_pat;
    if (stand_pat >= beta)
      return stand_pat;

    int scores[218] = {0};
    move_gen.generate_captures(board, ply);

    for (int i = 0; i < move_gen.move_lists[ply].count; i++)
    {
      scores[i] = score_move(board, move_gen.move_lists[ply].moves[i], ply);
    }

    for (int i = 0; i < move_gen.move_lists[ply].count; i++)
    {
      int best = i;
      for (int j = i + 1; j < move_gen.move_lists[ply].count; j++)
      {
        if (scores[j] > scores[best])
          best = j;
      }
      std::swap(scores[i], scores[best]);
      std::swap(move_gen.move_lists[ply].moves[i], move_gen.move_lists[ply].moves[best]);

      Move m = move_gen.move_lists[ply].moves[i];
      int captured_val = m.is_en_passant ? piece_vals[wPawn] : piece_vals[board.squares[m.to]];
      if (m.promotion_piece == EMPTY && stand_pat + captured_val + 200 < alpha)
        continue;
      board.make_move(m);
      if (move_gen.is_in_check(board, !board.is_white_to_move()))
      {
        board.unmake_move(m);
        continue;
      }
      int score = -quiescence(board, move_gen, -beta, -alpha, ply + 1, NO_EVAL, depth + 1, stop_flag);
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
