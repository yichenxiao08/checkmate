#pragma once

#include "board.h"
#include "moveGen.h"
#include "types.h"

extern Move killer_table[2][256];
extern int history_table[12][64];

void reset_killer_table();
void reset_history_table();
void reset_tt();

inline void reset_search()
{
  reset_killer_table();
  reset_history_table();
  reset_tt();
}

Move root_negamax(Board &board, MoveGenerator &move_gen, int depth);

int negamax(Board &board, MoveGenerator &move_gen, int alpha, int beta, int depth, int ply, bool can_null);

int score_move(Board &board, Move m, int ply);

int mvv_lva(Piece attack, Piece victim);

int quiescence(Board &board, MoveGenerator &move_gen, int alpha, int beta, int ply, int static_eval, int depth);

inline Move iterative_deepening(Board &board, MoveGenerator &move_gen, int max_depth)
{
  Move best_move;
  for (int i = 1; i <= max_depth; i++)
  {
    best_move = root_negamax(board, move_gen, i);
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
