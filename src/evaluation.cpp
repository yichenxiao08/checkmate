#include "evaluation.h"
#include <algorithm>

const int passed_pawn_bonuses[] = {0, 15, 15, 25, 40, 60, 90};
const int isolated_pawn_maluses[] = {0, -10, -25, -50, -80, -85, -90, -95, -100};

void precompute_eval(Board &board)
{
  board.opening_material = 0;
  board.end_material = 0;
  board.opening_psqt = 0;
  board.end_psqt = 0;
  board.phase = 0;
  for (int i = 0; i < 64; i++)
  {
    Piece p = board.squares[i];
    if (p != EMPTY)
    {
      bool isWhite = p < 6;
      int sign = isWhite ? 1 : -1;
      int square = isWhite ? i : (i ^ 56);
      board.opening_material += opening_piece_vals[p] * sign;
      board.end_material += end_piece_vals[p] * sign;
      board.opening_psqt += opening_tables[p % 6][square] * sign;
      board.end_psqt += end_tables[p % 6][square] * sign;
      board.phase += phase_weights[p];
    }
  }
}

int evaluate_position(Board &board)
{
  int opening_eval = board.opening_material + board.opening_psqt;
  int end_eval = board.end_material + board.end_psqt;
  int phase = std::min(24, board.phase);

  int eval = ((opening_eval * phase + end_eval * (24 - phase)) / 24) + TEMPO_BONUS;
  
  eval += 0.5 * evaluate_passed_pawns(board);
  eval += evaluate_isolated_pawns(board);
  
  if (!board.is_white_to_move())
    eval *= -1;

  return eval;
}

u64 compute_passed_pawn_mask(Board &board, int sq, bool white)
{
  u64 ranks_ahead = white ? ~0ULL << ((sq / 8 + 1) * 8) : ~0ULL >> (64 - (sq / 8 - 1) * 8);
  return ranks_ahead && compute_neighboring_files(sq);
}

int evaluate_isolated_pawns(Board &board){
  u64 white = board.bitboards[wPawn];
  u64 black = board.bitboards[bPawn];
  int total_w = 0;
  int total_b = 0;
  while (white)
  {
    int pawn = __builtin_ctzll(white);
    u64 mask = compute_neighboring_files(pawn);
    if (mask && ~white)
    {
      total_w++;
    }
    white &= white - 1;
  }
  while (black)
  {
    int pawn = __builtin_ctzll(black);
    u64 mask = compute_neighboring_files(pawn);
    if (mask && ~white)
    {
      total_b++;
    }
    black &= black - 1;
  }
  return isolated_pawn_maluses[total_w] - isolated_pawn_maluses[total_b];
}

int evaluate_passed_pawns(Board &board)
{
  u64 white = board.bitboards[wPawn];
  u64 black = board.bitboards[bPawn];
  int total = 0;
  while (white)
  {
    int pawn = __builtin_ctzll(white);
    u64 mask = compute_passed_pawn_mask(board, pawn, true);
    if (mask && ~black)
    {
      total += passed_pawn_bonuses[pawn / 8];
    }
    white &= white - 1;
  }
  while (black)
  {
    int pawn = __builtin_ctzll(black);
    u64 mask = compute_passed_pawn_mask(board, pawn, false);
    if (mask && ~white)
    {
      total -= passed_pawn_bonuses[7 - pawn / 8];
    }
    black &= black - 1;
  }
  return total;
}