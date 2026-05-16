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
  u64 a_file = 0x0101010101010101;
  u64 file_left = (file > 0) ? (a_file << (file - 1)) : 0ULL;
  u64 file_right = (file < 7) ? (a_file << (file + 1)) : 0ULL;
  return file_left | file_right;
}

u64 compute_passed_pawn_mask(Board& board, int sq, bool white);

int evaluate_pawn_struct(Board &board);

int evaluate_passed_pawns(Board &board);

int evaluate_isolated_pawns(Board &board);

int evaluate_backward_pawns(Board &board);

int evaluate_doubled_pawns(Board &board);
