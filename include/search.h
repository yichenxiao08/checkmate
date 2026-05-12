#pragma once

#include "board.h"
#include "moveGen.h"

Move rootNegamax(Board &board, MoveGenerator &moveGen, int depth);

int negamax(Board &board, MoveGenerator &moveGen, int alpha, int beta, int depth, int ply = 0);

int scoreMove(Board &board, Move m);

int mvv_lva(Piece attack, Piece victim);

int quiescence(Board &board, MoveGenerator &moveGen, int alpha, int beta, int ply = 0);

inline Move iterativeDeepening(Board &board, MoveGenerator &moveGen, int maxDepth)
{
  Move bestMove;
  for (int i = 1; i <= maxDepth; i++)
  {
    bestMove = rootNegamax(board, moveGen, i);
  }
  return bestMove;
}