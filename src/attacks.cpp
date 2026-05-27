#include "moveGen.h"
#include "board.h"
#include "bitboardMasks.h"
#include "magics.h"

void MoveGenerator::precompute_knight_attacks()
{
  for (int i = 0; i < 64; i++)
  {
    u64 knight_pos = 1ULL << i;
    u64 nn_e = (knight_pos << 17) & not_a_file;
    u64 ne_e = (knight_pos << 10) & not_ab_file;
    u64 se_e = (knight_pos >> 6) & not_ab_file;
    u64 ss_e = (knight_pos >> 15) & not_a_file;
    u64 ss_w = (knight_pos >> 17) & not_h_file;
    u64 sw_w = (knight_pos >> 10) & not_gh_file;
    u64 nw_w = (knight_pos << 6) & not_gh_file;
    u64 nn_w = (knight_pos << 15) & not_h_file;
    u64 knight_attacks = nn_e | ne_e | se_e | ss_e | ss_w | sw_w | nw_w | nn_w;
    knight_attack_table[i] = knight_attacks;
  }
}
void MoveGenerator::precompute_king_attacks()
{
  for (int i = 0; i < 64; i++)
  {
    u64 king_pos = 1ULL << i;
    u64 ne = (king_pos << 9) & not_a_file;
    u64 e = (king_pos << 1) & not_a_file;
    u64 se = (king_pos >> 7) & not_a_file;
    u64 s = (king_pos >> 8);
    u64 sw = (king_pos >> 9) & not_h_file;
    u64 w = (king_pos >> 1) & not_h_file;
    u64 nw = (king_pos << 7) & not_h_file;
    u64 n = (king_pos << 8);
    u64 king_attacks = ne | e | se | s | sw | w | nw | n;
    king_attack_table[i] = king_attacks;
  }
}
void MoveGenerator::precompute_white_pawn_attacks()
{
  for (int i = 0; i < 56; i++)
  {
    u64 pawn_pos = 1ULL << i;
    u64 ne = (pawn_pos << 9) & not_a_file;
    u64 nw = (pawn_pos << 7) & not_h_file;
    u64 white_pawn_attacks = ne | nw;
    white_pawn_attack_table[i] = white_pawn_attacks;
  }
}
void MoveGenerator::precompute_black_pawn_attacks()
{
  for (int i = 8; i < 64; i++)
  {
    u64 pawn_pos = 1ULL << i;
    u64 se = (pawn_pos >> 7) & not_a_file;
    u64 sw = (pawn_pos >> 9) & not_h_file;
    u64 black_pawn_attacks = se | sw;
    black_pawn_attack_table[i] = black_pawn_attacks;
  }
}
void MoveGenerator::precompute_rook_masks()
{
  for (int i = 0; i < 64; i++)
  {
    int x = i % 8;
    int y = i / 8;
    u64 mask = 0ULL;
    for (int j = 0; j < 7 - y - 1; j++)
    {
      mask |= (1ULL << (i + 8 * (j + 1)));
    }
    for (int j = 0; j < y - 1; j++)
    {
      mask |= (1ULL << (i - 8 * (j + 1)));
    }
    for (int j = 0; j < 7 - x - 1; j++)
    {
      mask |= (1ULL << (i + j + 1));
    }
    for (int j = 0; j < x - 1; j++)
    {
      mask |= (1ULL << (i - j - 1));
    }
    rook_masks[i] = mask;
  }
}
void MoveGenerator::precompute_bishop_masks()
{
  for (int i = 0; i < 64; i++)
  {
    int x = i % 8;
    int y = i / 8;
    int ne_bound = std::min(7 - x - 1, 7 - y - 1);
    int nw_bound = std::min(x - 1, 7 - y - 1);
    int se_bound = std::min(7 - x - 1, y - 1);
    int sw_bound = std::min(x - 1, y - 1);
    u64 mask = 0ULL;
    for (int j = 0; j < ne_bound; j++)
    {
      mask |= (1ULL << (i + 9 * (j + 1)));
    }
    for (int j = 0; j < nw_bound; j++)
    {
      mask |= (1ULL << (i + 7 * (j + 1)));
    }
    for (int j = 0; j < se_bound; j++)
    {
      mask |= (1ULL << (i - 7 * (j + 1)));
    }
    for (int j = 0; j < sw_bound; j++)
    {
      mask |= (1ULL << (i - 9 * (j + 1)));
    }
    bishop_masks[i] = mask;
  }
}
void MoveGenerator::precompute_rook_attacks()
{
  precompute_rook_masks();
  for (int i = 0; i < 64; i++)
  {
    u64 mask = rook_masks[i];
    u64 subset = 0;
    do
    {
      u64 attacks = calculate_rook_attacks(i, subset);
      u64 index = (subset * rook_magic[i]) >> (64 - rook_shifts[i]);
      rook_attack_table[i][index] = attacks;
      subset = (subset - mask) & mask;
    } while (subset != 0);
  }
}
void MoveGenerator::precompute_bishop_attacks()
{
  precompute_bishop_masks();
  for (int i = 0; i < 64; i++)
  {
    u64 mask = bishop_masks[i];
    u64 subset = 0;
    do
    {
      u64 attacks = calculate_bishop_attacks(i, subset);
      u64 index = (subset * bishop_magic[i]) >> (64 - bishop_shifts[i]);
      bishop_attack_table[i][index] = attacks;
      subset = (subset - mask) & mask;
    } while (subset != 0);
  }
}
u64 MoveGenerator::calculate_rook_attacks(int sq, u64 blockers)
{
  u64 attacks = 0ULL;
  int file = sq % 8;
  int rank = sq / 8;
  for (int r = rank + 1; r <= 7; r++)
  {
    attacks |= (1ULL << (r * 8 + file));
    if (blockers & (1ULL << (r * 8 + file)))
      break;
  }
  for (int r = rank - 1; r >= 0; r--)
  {
    attacks |= (1ULL << (r * 8 + file));
    if (blockers & (1ULL << (r * 8 + file)))
      break;
  }
  for (int f = file + 1; f <= 7; f++)
  {
    attacks |= (1ULL << (rank * 8 + f));
    if (blockers & (1ULL << (rank * 8 + f)))
      break;
  }
  for (int f = file - 1; f >= 0; f--)
  {
    attacks |= (1ULL << (rank * 8 + f));
    if (blockers & (1ULL << (rank * 8 + f)))
      break;
  }
  return attacks;
}
u64 MoveGenerator::calculate_bishop_attacks(int sq, u64 blockers)
{
  u64 attacks = 0ULL;
  int file = sq % 8;
  int rank = sq / 8;

  int ne_bound = std::min(7 - rank, 7 - file);
  int nw_bound = std::min(rank, 7 - file);
  int se_bound = std::min(7 - rank, file);
  int sw_bound = std::min(rank, file);
  u64 mask = 0ULL;
  for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++)
  {
    attacks |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f)))
      break;
  }
  for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--)
  {
    attacks |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f)))
      break;
  }
  for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++)
  {
    attacks |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f)))
      break;
  }
  for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--)
  {
    attacks |= (1ULL << (r * 8 + f));
    if (blockers & (1ULL << (r * 8 + f)))
      break;
  }
  return attacks;
}
u64 MoveGenerator::get_rook_attacks(int sq, Board &board)
{
  u64 blockers = (board.white_pieces | board.black_pieces) & rook_masks[sq];
  u64 index = (blockers * rook_magic[sq]) >> (64 - rook_shifts[sq]);
  u64 max_index = 1ULL << rook_shifts[sq];
  return rook_attack_table[sq][index];
}
u64 MoveGenerator::get_bishop_attacks(int sq, Board &board)
{
  u64 blockers = (board.white_pieces | board.black_pieces) & bishop_masks[sq];
  u64 index = (blockers * bishop_magic[sq]) >> (64 - bishop_shifts[sq]);
  return bishop_attack_table[sq][index];
}
u64 MoveGenerator::get_queen_attacks(int sq, Board &board)
{
  return get_rook_attacks(sq, board) | get_bishop_attacks(sq, board);
}
bool MoveGenerator::is_attacked(Board &board, bool white, int sq)
{
  if (white)
  {
    if (knight_attack_table[sq] & board.bitboards[bKnight])
      return true;
    if (king_attack_table[sq] & board.bitboards[bKing])
      return true;
    if (sq < 56)
    {
      u64 potential_black_pawns = ((1ULL << (sq + 7)) & not_h_file) | ((1ULL << (sq + 9)) & not_a_file);
      if (board.bitboards[bPawn] & potential_black_pawns)
        return true;
    }
    if (get_rook_attacks(sq, board) & (board.bitboards[bRook] | board.bitboards[bQueen]))
      return true;
    if (get_bishop_attacks(sq, board) & (board.bitboards[bBishop] | board.bitboards[bQueen]))
      return true;
  }
  else
  {
    if (knight_attack_table[sq] & board.bitboards[wKnight])
      return true;
    if (king_attack_table[sq] & board.bitboards[wKing])
      return true;
    if (sq >= 8)
    {
      u64 potential_white_pawns = ((1ULL << (sq - 7)) & not_a_file) | ((1ULL << (sq - 9)) & not_h_file);
      if (board.bitboards[wPawn] & potential_white_pawns)
        return true;
    }
    if (get_rook_attacks(sq, board) & (board.bitboards[wRook] | board.bitboards[wQueen]))
      return true;
    if (get_bishop_attacks(sq, board) & (board.bitboards[wBishop] | board.bitboards[wQueen]))
      return true;
  }
  return false;
}
bool MoveGenerator::is_in_check(Board &board, bool white)
{
  int king_sq = __builtin_ctzll(white ? board.bitboards[wKing] : board.bitboards[bKing]);

  if (white)
  {
    if (knight_attack_table[king_sq] & board.bitboards[bKnight])
      return true;
    if (king_attack_table[king_sq] & board.bitboards[bKing])
      return true;
    u64 potential_black_pawns = 0;
    if (king_sq < 56)
    {
      potential_black_pawns |= ((1ULL << (king_sq + 7)) & not_h_file) | ((1ULL << (king_sq + 9)) & not_a_file);
      if (board.bitboards[bPawn] & potential_black_pawns)
        return true;
    }
    if (get_rook_attacks(king_sq, board) & (board.bitboards[bRook] | board.bitboards[bQueen]))
      return true;
    if (get_bishop_attacks(king_sq, board) & (board.bitboards[bBishop] | board.bitboards[bQueen]))
      return true;
  }
  else
  {
    if (knight_attack_table[king_sq] & board.bitboards[wKnight])
      return true;
    if (king_attack_table[king_sq] & board.bitboards[wKing])
      return true;
    u64 potential_white_pawns = 0;
    if (king_sq >= 8)
    {
      potential_white_pawns |= ((1ULL << (king_sq - 7)) & not_a_file) | ((1ULL << (king_sq - 9)) & not_h_file);
      if (board.bitboards[wPawn] & potential_white_pawns)
        return true;
    }
    if (get_rook_attacks(king_sq, board) & (board.bitboards[wRook] | board.bitboards[wQueen]))
      return true;
    if (get_bishop_attacks(king_sq, board) & (board.bitboards[wBishop] | board.bitboards[wQueen]))
      return true;
  }
  return false;
}
