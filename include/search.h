#pragma once

#include "board.h"
#include "moveGen.h"
#include "types.h"
#include <atomic>
#include <chrono>

extern Move killer_table[2][256];
extern int history_table[12][64];
extern u64 repetition_table[1024];
extern int repetition_count;

void reset_killer_table();
void reset_history_table();
void reset_tt();

inline bool is_repetition(const Board &board)
{
  int stop = repetition_count - 1 - board.half_move_count;
  if (stop < 0) stop = 0;
  for (int i = repetition_count - 3; i >= stop; i -= 2)
  {
    if (repetition_table[i] == board.hash) return true;
  }
  return false;
}

inline void reset_search()
{
  reset_killer_table();
  reset_history_table();
  reset_tt();
}

Move root_negamax(Board &board, MoveGenerator &move_gen, int depth, std::atomic<bool> &stop_flag);

int negamax(Board &board, MoveGenerator &move_gen, int alpha, int beta, int depth, int ply, bool can_null, std::atomic<bool> &stop_flag);

int score_move(Board &board, Move m, int ply);

int mvv_lva(Piece attack, Piece victim);

int quiescence(Board &board, MoveGenerator &move_gen, int alpha, int beta, int ply, int static_eval, int depth, std::atomic<bool> &stop_flag);

inline Move iterative_deepening(Board &board, MoveGenerator &move_gen, int max_depth, std::atomic<bool> &stop_flag, int &depth_search, std::chrono::steady_clock::time_point start, int soft_limit)
{
  Move best_move;
  for (int i = 1; i <= max_depth; i++)
  {
    Move m = root_negamax(board, move_gen, i, stop_flag);
    if (i == 1 || !stop_flag.load())
    {
      best_move = m;
      depth_search = i;
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    if (elapsed >= soft_limit){
      break;
    }
    for (int p = 0; p < 12; p++)
    {
      for (int s = 0; s < 64; s++)
      {
        history_table[p][s] /= 2;
      }
    }
  }
  return best_move;
}
