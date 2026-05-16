#pragma once
#include "board.h"
#include "evalConstants.h"
#include <algorithm>

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

inline u64 compute_neighboring_files(int sq){
  int file = sq % 8;
  u64 a_file = 0x1010101010101010;
  u64 file_on = a_file << file;
  u64 file_left = a_file << std::max(0, file - 1);
  u64 file_right = a_file << std::min(7, file + 1);
  return file_on && file_left && file_right;
}

u64 compute_passed_pawn_mask(Board& board, int sq, bool white);

int evaluate_passed_pawns(Board &board);

int evaluate_isolated_pawns(Board &board);

int evaluate_backward_pawns(Board &board);

int evaluate_doubled_pawns(Board &board);
