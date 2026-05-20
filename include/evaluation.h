#pragma once
#include "board.h"
#include "evalConstants.h"
#include "transposition.h"
#include <algorithm>

const u64 a_file = 0x0101010101010101;

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

void precompute_eval(Board &board);

int evaluate_position(Board &board);

inline u64 compute_neighboring_files(int sq)
{
  int file = sq % 8;
  u64 file_left = (file > 0) ? (a_file << (file - 1)) : 0ULL;
  u64 file_right = (file < 7) ? (a_file << (file + 1)) : 0ULL;
  return file_left | file_right;
}

inline bool is_semi_open_file(Board &board, int sq)
{
  int file = sq % 8;
  u64 file_on = a_file << file;
  if ((board.bitboards[wPawn] & file_on) == 0)
    return true;
  if ((board.bitboards[bPawn] & file_on) == 0)
    return true;
  return false;
}

inline bool is_semi_open_file(Board &board, u64 file, bool white)
{
  if (((board.bitboards[wPawn] & file) == 0) && white)
    return true;
  if (((board.bitboards[bPawn] & file) == 0) && !white)
    return true;
  return false;
}

inline bool is_rook_on_semi(Board &board, int sq, bool white)
{
  int file = sq % 8;
  u64 file_on = a_file << file;
  if ((board.bitboards[wPawn] & file_on) == 0 && white)
    return true;
  if ((board.bitboards[bPawn] & file_on) == 0 && !white)
    return true;
  return false;
}

inline bool is_rook_on_seventh(Board &board, int sq, bool white)
{
  int rank = sq / 8;
  return white ? rank == 6 : rank == 1;
}

inline bool is_open_file(Board &board, int sq)
{
  int file = sq % 8;
  u64 file_on = a_file << file;
  if (((board.bitboards[wPawn] & file_on) == 0) && ((board.bitboards[bPawn] & file_on) == 0))
    return true;
  return false;
}

inline bool is_open_file(Board &board, u64 file)
{
  if (((board.bitboards[wPawn] & file) == 0) && ((board.bitboards[bPawn] & file) == 0))
    return true;
  return false;
}

u64 compute_passed_pawn_mask(Board &board, int sq, bool white);

int evaluate_pawn_struct(Board &board, int phase);

int evaluate_passed_pawns(Board &board);

int evaluate_isolated_pawns(Board &board);

int evaluate_backward_pawns(Board &board);

int evaluate_doubled_pawns(Board &board);

int evaluate_rooks(Board &board, int phase);

int knight_outposts(Board &board, int phase);

inline int bishop_pair(Board &board, int phase)
{
  int white_count = 0;
  int black_count = 0;
  int opening_eval = 0;
  int end_eval = 0;
  u64 white = board.bitboards[wBishop];
  u64 black = board.bitboards[bBishop];
  while (white)
  {
    white_count++;
    white &= white - 1;
  }
  while (black)
  {
    black_count++;
    black &= black - 1;
  }
  if (white_count >= 2)
  {
    opening_eval += 20;
    end_eval += 50;
  }
  if (black_count >= 2)
  {
    opening_eval -= 20;
    end_eval -= 50;
  }
  return (opening_eval * phase + end_eval * (24 - phase)) / 24;
}

int evaluate_king_safety(Board &board, int phase);

int pawn_shield(Board &board, int sq, bool white, u64 file_on, u64 file_close, u64 file_far);

int king_zone(Board &board, int sq, bool white);