#pragma once

#include "board.h"
#include "moveGen.h"

extern Move killer_table[2][256];
extern Move history_table[12][64];

Move root_negamax(Board &board, MoveGenerator &move_gen, int depth);

int negamax(Board &board, MoveGenerator &move_gen, int alpha, int beta, int depth, int ply = 0, bool can_null);

int score_move(Board &board, Move m, int ply);

int mvv_lva(Piece attack, Piece victim);

int quiescence(Board &board, MoveGenerator &move_gen, int alpha, int beta, int ply = 0, int static_eval);

inline Move iterative_deepening(Board &board, MoveGenerator &move_gen, int max_depth)
{
  Move best_move;
  for (int i = 1; i <= max_depth; i++)
  {
    best_move = root_negamax(board, move_gen, i);
  }
  return best_move;
}
