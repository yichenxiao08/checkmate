#include "nnue.h"

Matrix encode_position(Board &board)
{
  Matrix pos(768, 1);
  for (int i = 0; i < 12; i++)
  {
    u64 bitboard = board.bitboards[i];
    while (bitboard)
    {
      int square = __builtin_ctzll(bitboard);
      int index = i * 64 + square;
      pos.at(index, 0) = 1.0f;
      bitboard &= bitboard - 1;
    }
  }
  return pos;
}