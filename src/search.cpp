#include "search.h"
#include "transposition.h"
#include "search_constants.h"
#include "evaluation.h"
#include <climits>
#include <algorithm>

Move killer_table[2][256];

int score_move(Board &board, Move m, int ply)
{
  if (board.squares[m.to] != EMPTY)
  {

    return mvv_lva(board.squares[m.from], board.squares[m.to]);
  }
  if(board.is_same_move(m, killer_table[0][ply])){
    return 90000;
  }
  if(board.is_same_move(m, killer_table[1][ply])){
    return 80000;
  }
  else
    return 0;
}
int mvv_lva(Piece attack, Piece victim) { return piece_vals[victim] - piece_vals[attack]; }

int negamax(Board &board, MoveGenerator &move_gen, int alpha, int beta, int depth, int ply)
{
  int original_alpha = alpha;
  Move best_move;

  int entry_score;
  Move entry_move;
  if (probe_entry(board.hash, depth, alpha, beta, ply, entry_score, entry_move))
  {
    return entry_score;
  }
  if (depth == 0)
    return quiescence(board, move_gen, alpha, beta, ply);
  int greatest_value = -INF;
  move_gen.generate_moves(board, ply);
  int scores[218] = {0};
  for (int i = 0; i < move_gen.move_lists[ply].count; i++)
  {
    if (board.is_same_move(move_gen.move_lists[ply].moves[i], entry_move)) scores[i] = INT_MAX;
    else
      scores[i] = score_move(board, move_gen.move_lists[ply].moves[i], ply);
  }
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
    int score = -negamax(board, move_gen, -beta, -alpha, depth - 1, ply + 1);
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
      if(board.squares[m.to] == EMPTY){
        killer_table[1][ply] = killer_table[0][ply];
        killer_table[0][ply] = m;
      }
      board.unmake_move(m);
      break;
    }
    board.unmake_move(m);
  }
  if (greatest_value == INT_MIN)
  {
    if (move_gen.is_in_check(board, board.is_white_to_move()))
    {
      return -MATE_THRESHOLD + ply;
    }
    else
      return 0;
  }
  int node_type;
  if (greatest_value <= original_alpha)
    node_type = UPPER;
  else if (greatest_value >= beta)
    node_type = LOWER;
  else
    node_type = EXACT;
  store_entry(board.hash, greatest_value, depth, best_move, node_type, ply);
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
    int score = -negamax(board, move_gen, -INF, INF, depth - 1, 1);
    board.unmake_move(m);

    if (score > best_score)
    {
      best_score = score;
      best_move = m;
    }
  }
  return best_move;
}

int quiescence(Board &board, MoveGenerator &move_gen, int alpha, int beta, int ply)
{
  bool is_in_check = move_gen.is_in_check(board, board.is_white_to_move());
  int greatest_val = INT_MIN;
  if (!is_in_check)
  {
    int stand_pat = evaluate(board);
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
      int score = -quiescence(board, move_gen, -beta, -alpha, ply + 1);
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
      int score = -quiescence(board, move_gen, -beta, -alpha, ply + 1);
      board.unmake_move(m);
      if (score >= beta)
        return score;
      if (score > greatest_val)
        greatest_val = score;
      if (score > alpha)
        alpha = score;
    }
  }
  if (greatest_val == INT_MIN && is_in_check)
    return -MATE_THRESHOLD + ply;
  return greatest_val;
}
