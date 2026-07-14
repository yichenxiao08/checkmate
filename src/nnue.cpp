#include "nnue.h"

NNUE::NNUE() : network(768, 256, {32, 32, 1}) {}

Matrix NNUE::encode_position(Board &board, bool white)
{
  Matrix pos(768, 1);
  for (int i = 0; i < 12; i++)
  {
    u64 bitboard = board.bitboards[i];
    while (bitboard)
    {
      int square = __builtin_ctzll(bitboard);
      int black_square = square ^ 0b111000;
      int index = white ? i * 64 + square : (i + 6) % 12 * 64 + black_square;
      pos.at(index, 0) = 1.0f;
      bitboard &= bitboard - 1;
    }
  }
  return pos;
}

void NNUE::evaluate(Board &board)
{
  Matrix white_inputs = encode_position(board, true);
  Matrix black_inputs = encode_position(board, false);

  
}