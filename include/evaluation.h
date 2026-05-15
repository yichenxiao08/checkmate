#pragma once
#include "board.h"
#include "evalConstants.h"


inline int count_pieces(Piece p, Board &board)
{
  int count = 0;
  u64 bitboard = board.bitboards[p];
  while (bitboard > 0)
  {
    count = count + 1;
    bitboard = bitboard & (bitboard - 1);
  }
  return count;
}

void precompute_eval(Board& board);

int evaluate_position(Board& board);

