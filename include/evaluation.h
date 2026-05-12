#pragma once
#include "board.h"

const int piece_vals[] = {100, 300, 310, 500, 900, -1, 100, 300, 310, 500, 900, -1};

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

int material_odds(Board &board);
int knight_odds(Board &board);

inline int evaluate(Board &board)
{
  int m = material_odds(board);
  return board.white_to_move ? m : -m;
}
