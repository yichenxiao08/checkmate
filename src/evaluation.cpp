#include "evaluation.h"
#include "bitboardMasks.h"

const int opening_passed_pawn_bonuses[] = {0, 5, 10, 20, 35, 50, 60};
const int end_passed_pawn_bonuses[] = {0, 15, 15, 25, 40, 70, 120};
const int opening_isolated_pawn_maluses[] = {0, -10, -25, -50, -80, -85, -90, -95, -100};
const int end_isolated_pawn_maluses[] = {0, -20, -45, -80, -120, -130, -140, -150, -160};
const int opening_knight_outpost_bonuses[] = {0, 0, 0, 5, 15, 25, 15, 0};
const int end_knight_outpost_bonuses[] = {0, 0, 0, 3, 5, 10, 5, 0};
const int attacker_count_bonus[] = {0, 0, 50, 75, 88, 94, 97, 99};

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

int evaluate_position(Board &board, MoveGenerator &mg)
{
  int opening_eval = board.opening_material + board.opening_psqt;
  int end_eval = board.end_material + board.end_psqt;
  int phase = std::min(24, board.phase);

  int eval = ((opening_eval * phase + end_eval * (24 - phase)) / 24);

  eval += evaluate_pawn_struct(board, phase);
  eval += evaluate_rooks(board, phase);
  eval += bishop_pair(board, phase);
  eval += knight_outposts(board, phase);
  eval += evaluate_king_safety(board, phase, mg);

  if (!board.is_white_to_move())
    eval *= -1;

  return eval + TEMPO_BONUS;
}

u64 compute_passed_pawn_mask(Board &board, int sq, bool white)
{
  u64 file_on = a_file << (sq % 8);
  u64 ranks_ahead = white ? ~0ULL << ((sq / 8 + 1) * 8) : ~0ULL >> (64 - (sq / 8) * 8);
  return ranks_ahead & (compute_neighboring_files(sq) | file_on);
}

int evaluate_pawn_struct(Board &board, int phase)
{
  PawnEntry &entry = pawn_table[board.pawns_hash & (pawn_table_size - 1)];
  if (entry.key == board.pawns_hash)
  {
    return entry.score;
  }

  u64 white = board.bitboards[wPawn];
  u64 white_pawns = white;
  u64 black = board.bitboards[bPawn];
  u64 black_pawns = black;
  int opening_eval = 0;
  int end_eval = 0;
  int total_w_isolated = 0;
  int total_b_isolated = 0;
  int opening_passed_eval = 0;
  int end_passed_eval = 0;
  int doubled = 0;
  while (white_pawns)
  {
    int pawn = __builtin_ctzll(white_pawns);
    u64 neighbors = compute_neighboring_files(pawn);
    if ((neighbors & white) == 0)
    {
      total_w_isolated++;
    }
    white_pawns &= white_pawns - 1;
    u64 passed_mask = compute_passed_pawn_mask(board, pawn, true);
    if ((passed_mask & black) == 0)
    {
      opening_passed_eval += opening_passed_pawn_bonuses[pawn / 8];
      end_passed_eval += end_passed_pawn_bonuses[pawn / 8];
    }
    u64 file = 0x0101010101010101 << (pawn % 8);
    u64 file_without_pawn = file & ~(1ULL << pawn);
    if (file_without_pawn & white)
    {
      doubled++;
    }
  }
  while (black_pawns)
  {
    int pawn = __builtin_ctzll(black_pawns);
    u64 neighbors = compute_neighboring_files(pawn);
    if ((neighbors & black) == 0)
    {
      total_b_isolated++;
    }
    u64 passed_mask = compute_passed_pawn_mask(board, pawn, false);
    if ((passed_mask & white) == 0)
    {
      opening_passed_eval -= opening_passed_pawn_bonuses[7 - pawn / 8];
      end_passed_eval -= end_passed_pawn_bonuses[7 - pawn / 8];
    }
    black_pawns &= black_pawns - 1;
    u64 file = 0x0101010101010101 << (pawn % 8);
    u64 file_without_pawn = file & ~(1ULL << pawn);
    if (file_without_pawn & black)
    {
      doubled--;
    }
  }
  opening_eval += (opening_isolated_pawn_maluses[total_w_isolated] - opening_isolated_pawn_maluses[total_b_isolated]);
  end_eval += (end_isolated_pawn_maluses[total_w_isolated] - end_isolated_pawn_maluses[total_b_isolated]);
  opening_eval += opening_passed_eval / 2;
  end_eval += end_passed_eval / 2;
  opening_eval -= doubled * 10;
  end_eval -= doubled * 25;

  int eval = ((opening_eval * phase + end_eval * (24 - phase)) / 24);
  store_pawn_entry(board.pawns_hash, eval);

  return eval;
}

int evaluate_rooks(Board &board, int phase)
{
  u64 white = board.bitboards[wRook];
  u64 black = board.bitboards[bRook];
  int opening_eval = 0;
  int end_eval = 0;
  while (white)
  {
    int sq = __builtin_ctzll(white);
    if (is_open_file(board, sq))
    {
      opening_eval += 25;
      end_eval += 15;
    }
    else if (is_rook_on_semi(board, sq, true))
    {
      opening_eval += 15;
      end_eval += 5;
    }
    // if(is_rook_on_seventh(board, sq, true)){
    //   opening_eval += 10;
    //   end_eval += 30;
    // }
    white &= white - 1;
  }
  while (black)
  {
    int sq = __builtin_ctzll(black);
    if (is_open_file(board, sq))
    {
      opening_eval -= 25;
      end_eval -= 15;
    }
    else if (is_rook_on_semi(board, sq, false))
    {
      opening_eval -= 15;
      end_eval -= 5;
    }
    // if (is_rook_on_seventh(board, sq, false))
    // {
    //   opening_eval -= 10;
    //   end_eval -= 30;
    // }
    black &= black - 1;
  }
  int eval = ((opening_eval * phase + end_eval * (24 - phase)) / 24);
  return eval;
}

int knight_outposts(Board &board, int phase)
{
  u64 white = board.bitboards[wKnight];
  u64 black = board.bitboards[bKnight];
  int opening_eval = 0;
  int end_eval = 0;
  while (white)
  {
    int sq = __builtin_ctzll(white);
    if (sq / 8 >= 3)
    {
      u64 ranks_ahead = ~0ULL << ((sq / 8 + 1) * 8);
      u64 enemy_pawns = compute_neighboring_files(sq) & ranks_ahead & board.bitboards[bPawn];
      if (enemy_pawns == 0)
      {
        opening_eval += opening_knight_outpost_bonuses[sq / 8];
        end_eval += end_knight_outpost_bonuses[sq / 8];
        if ((sq % 8 != 0 && sq % 8 != 7) && (board.squares[sq - 9] == wPawn || board.squares[sq - 7] == wPawn))
        {
          opening_eval += opening_knight_outpost_bonuses[sq / 8] / 2;
          end_eval += end_knight_outpost_bonuses[sq / 8] / 2;
        }
        else if ((sq % 8 != 0) && (board.squares[sq - 9] == wPawn))
        {
          opening_eval += opening_knight_outpost_bonuses[sq / 8] / 2;
          end_eval += end_knight_outpost_bonuses[sq / 8] / 2;
        }
        else if ((sq % 8 != 7) && (board.squares[sq - 7] == wPawn))
        {
          opening_eval += opening_knight_outpost_bonuses[sq / 8] / 2;
          end_eval += end_knight_outpost_bonuses[sq / 8] / 2;
        }
      }
    }
    white &= white - 1;
  }
  while (black)
  {
    int sq = __builtin_ctzll(black);
    if (sq / 8 <= 4)
    {
      int idx = 7 - sq / 8;
      u64 ranks_ahead = (1ULL << (sq / 8 * 8)) - 1;
      u64 enemy_pawns = compute_neighboring_files(sq) & ranks_ahead & board.bitboards[wPawn];
      if (enemy_pawns == 0)
      {
        opening_eval -= opening_knight_outpost_bonuses[idx];
        end_eval -= end_knight_outpost_bonuses[idx];
        if ((sq % 8 != 0 && sq % 8 != 7) && (board.squares[sq + 9] == bPawn || board.squares[sq + 7] == bPawn))
        {
          opening_eval -= opening_knight_outpost_bonuses[idx] / 2;
          end_eval -= end_knight_outpost_bonuses[idx] / 2;
        }
        else if ((sq % 8 != 0) && (board.squares[sq + 7] == bPawn))
        {
          opening_eval -= opening_knight_outpost_bonuses[idx] / 2;
          end_eval -= end_knight_outpost_bonuses[idx] / 2;
        }
        else if ((sq % 8 != 7) && (board.squares[sq + 9] == bPawn))
        {
          opening_eval -= opening_knight_outpost_bonuses[idx] / 2;
          end_eval -= end_knight_outpost_bonuses[idx] / 2;
        }
      }
    }
    black &= black - 1;
  }
  return (opening_eval * phase + end_eval * (24 - phase)) / 24;
}

int pawn_shield(Board &board, int sq, bool white, u64 file_on, u64 file_close, u64 file_far)
{
  int eval = 0;
  if (white)
  {
    u64 pawns_on = file_on & board.bitboards[wPawn];
    u64 pawns_far = file_far & board.bitboards[wPawn];
    u64 pawns_close = file_close & board.bitboards[wPawn];
    if (pawns_on == 0)
      eval -= 30;
    else
    {
      int pawn = __builtin_ctzll(pawns_on);
      int rank = pawn / 8 - (sq + 8) / 8;
      if (rank == 1)
        eval -= 10;
      else if (rank == 2)
        eval -= 20;
    }
    if (file_close != 0)
    {
      if (pawns_close == 0)
        eval -= 15;
      else
      {
        int pawn = __builtin_ctzll(pawns_close);
        int rank = pawn / 8 - (sq + 8) / 8;
        if (rank == 1)
          eval -= 5;
        else if (rank == 2)
          eval -= 10;
      }
    }
    if (file_far != 0)
    {
      if (pawns_far == 0)
        eval -= 15;
      else
      {
        int pawn = __builtin_ctzll(pawns_far);
        int rank = pawn / 8 - (sq + 8) / 8;
        if (rank == 1)
          eval -= 10;
        else if (rank == 2)
          eval -= 15;
      }
    }
  }
  else
  {
    u64 pawns_on = file_on & board.bitboards[bPawn];
    u64 pawns_far = file_far & board.bitboards[bPawn];
    u64 pawns_close = file_close & board.bitboards[bPawn];
    if (pawns_on == 0)
      eval += 30;
    else
    {
      int pawn = 63 - __builtin_clzll(pawns_on);
      int rank = (sq - 8) / 8 - pawn / 8;
      if (rank == 1)
        eval += 10;
      else if (rank == 2)
        eval += 20;
    }
    if (file_close != 0)
    {
      if (pawns_close == 0)
        eval += 15;
      else
      {
        int pawn = 63 - __builtin_clzll(pawns_close);
        int rank = (sq - 8) / 8 - pawn / 8;
        if (rank == 1)
          eval += 5;
        else if (rank == 2)
          eval += 10;
      }
    }
    if (file_far != 0)
    {
      if (pawns_far == 0)
        eval += 15;
      else
      {
        int pawn = 63 - __builtin_clzll(pawns_far);
        int rank = (sq - 8) / 8 - pawn / 8;
        if (rank == 1)
          eval += 10;
        else if (rank == 2)
          eval += 15;
      }
    }
  }
  return eval;
}

int evaluate_king_safety(Board &board, int phase, MoveGenerator &mg)
{
  // castling rights check
  int white = __builtin_ctzll(board.bitboards[wKing]);
  int eval = 0;
  if (board.white_castled)
    eval += 25;
  else if (!board.white_castled && ((board.castling_rights & (wK | wQ)) == 0) && (white % 8 == 3 || white % 8 == 4))
    eval -= 50;

  bool white_kingside = (white % 8 >= 4) ? true : false;
  u64 file_on = a_file << (white % 8);
  u64 file_left = 0ULL;
  u64 file_right = 0ULL;
  if (white % 8 > 0)
  {
    file_left = a_file << (white % 8 - 1);
  }
  if (white % 8 < 7)
  {
    file_right = a_file << (white % 8 + 1);
  }

  // open files and shield
  if (white_kingside)
  {
    if (white / 8 >= 6)
      eval -= 25;
    else if (white % 8 <= 2 || white % 8 >= 5)
    {
      eval += pawn_shield(board, white, true, file_on, file_left, file_right);
    }
    if (is_open_file(board, file_left))
      eval -= 20;
    else if (is_semi_open_file(board, file_left, true))
      eval -= 10;
    if (is_open_file(board, file_on))
      eval -= 40;
    else if (is_semi_open_file(board, file_on, true))
      eval -= 30;
    if (is_open_file(board, file_right) && file_right != 0)
      eval -= 30;
    else if (is_semi_open_file(board, file_right, true) && file_right != 0)
      eval -= 20;
  }
  else
  {
    if (white % 8 <= 2 || white % 8 >= 5)
      eval += pawn_shield(board, white, true, file_on, file_right, file_left);
    if (is_open_file(board, file_left) && file_left != 0)
      eval -= 30;
    else if (is_semi_open_file(board, file_left, true) && file_left != 0)
      eval -= 20;
    if (is_open_file(board, file_on))
      eval -= 40;
    else if (is_semi_open_file(board, file_on, true))
      eval -= 30;
    if (is_open_file(board, file_right))
      eval -= 20;
    else if (is_semi_open_file(board, file_right, true))
      eval -= 10;
  }
  int black = __builtin_ctzll(board.bitboards[bKing]);
  if (board.black_castled)
    eval -= 25;
  else if (!board.black_castled && ((board.castling_rights & (bK | bQ)) == 0) && (black % 8 == 3 || black % 8 == 4))
    eval += 50;

  bool black_kingside = (black % 8 >= 4) ? true : false;
  file_on = a_file << (black % 8);
  file_left = 0ULL;
  file_right = 0ULL;
  if (black % 8 > 0)
  {
    file_left = a_file << (black % 8 - 1);
  }
  if (black % 8 < 7)
  {
    file_right = a_file << (black % 8 + 1);
  }

  // open files and shield
  if (black_kingside)
  {
    if (black / 8 <= 1)
      eval += 25;
    else if (black % 8 <= 2 || black % 8 >= 5)
    {
      eval += pawn_shield(board, black, false, file_on, file_left, file_right);
    }
    if (is_open_file(board, file_left))
      eval += 20;
    else if (is_semi_open_file(board, file_left, false))
      eval += 10;
    if (is_open_file(board, file_on))
      eval += 40;
    else if (is_semi_open_file(board, file_on, false))
      eval += 30;
    if (is_open_file(board, file_right) && file_right != 0)
      eval += 30;
    else if (is_semi_open_file(board, file_right, false) && file_right != 0)
      eval += 20;
  }
  else
  {
    if (black % 8 <= 2 || black % 8 >= 5)
      eval += pawn_shield(board, black, false, file_on, file_right, file_left);
    if (is_open_file(board, file_left) && file_left != 0)
      eval += 30;
    else if (is_semi_open_file(board, file_left, false) && file_left != 0)
      eval += 20;
    if (is_open_file(board, file_on))
      eval += 40;
    else if (is_semi_open_file(board, file_on, false))
      eval += 30;
    if (is_open_file(board, file_right))
      eval += 20;
    else if (is_semi_open_file(board, file_right, false))
      eval += 10;
  }

  eval -= king_zone(board, white, true, mg);
  eval += king_zone(board, black, false, mg);

  return (eval * phase) / 24;
}

int king_zone(Board &board, int sq, bool is_white, MoveGenerator &mg)
{
  int file = sq % 8;
  u64 file_on = a_file << file;
  u64 file_left = file > 0 ? a_file << (file - 1) : 0ULL;
  u64 file_right = file < 7 ? a_file << (file + 1) : 0ULL;
  u64 rank = first_rank << ((sq / 8) * 8);
  u64 rank_above = (sq / 8) < 7 ? first_rank << ((sq / 8 + 1) * 8) : 0ULL;
  u64 rank_below = (sq / 8) > 0 ? first_rank << ((sq / 8 - 1) * 8) : 0ULL;
  int eval = 0;
  int attacker_count = 0;
  if (is_white)
  {
    u64 rank_double = (sq / 8) < 6 ? first_rank << ((sq / 8 + 2) * 8) : 0ULL;
    u64 zone = (file_on | file_left | file_right) & (rank | rank_above | rank_below | rank_double);
    u64 knights = board.bitboards[bKnight];
    while (knights)
    {
      int square = __builtin_ctzll(knights);
      u64 attacked_squares = mg.knight_attack_table[square] & zone;
      int count = __builtin_popcountll(attacked_squares);
      eval += count * 2;
      attacker_count = count != 0 ? attacker_count + 1 : attacker_count;
      knights &= knights - 1;
    }

    u64 bishops = board.bitboards[bBishop];
    while (bishops)
    {
      int square = __builtin_ctzll(bishops);
      u64 attacked_squares = mg.get_bishop_attacks(square, board) & zone;
      int count = __builtin_popcountll(attacked_squares);
      eval += count * 2;
      attacker_count = count != 0 ? attacker_count + 1 : attacker_count;
      bishops &= bishops - 1;
    }

    u64 rooks = board.bitboards[bRook];
    while (rooks)
    {
      int square = __builtin_ctzll(rooks);
      u64 attacked_squares = mg.get_rook_attacks(square, board) & zone;
      int count = __builtin_popcountll(attacked_squares);
      eval += count * 3;
      attacker_count = count != 0 ? attacker_count + 1 : attacker_count;
      rooks &= rooks - 1;
    }

    u64 queen = board.bitboards[bQueen];
    while (queen)
    {
      int square = __builtin_ctzll(queen);
      u64 attacked_squares = mg.get_queen_attacks(square, board) & zone;
      int count = __builtin_popcountll(attacked_squares);
      eval += count * 5;
      attacker_count = count != 0 ? attacker_count + 1 : attacker_count;
      queen &= queen - 1;
    }
  }
  else
  {
    u64 rank_double = (sq / 8) > 1 ? first_rank << ((sq / 8 - 2) * 8) : 0ULL;
    u64 zone = (file_on | file_left | file_right) & (rank | rank_above | rank_below | rank_double);

    u64 knights = board.bitboards[wKnight];
    while (knights)
    {
      int square = __builtin_ctzll(knights);
      u64 attacked_squares = mg.knight_attack_table[square] & zone;
      int count = __builtin_popcountll(attacked_squares);
      eval += count * 2;
      attacker_count = count != 0 ? attacker_count + 1 : attacker_count;
      knights &= knights - 1;
    }

    u64 bishops = board.bitboards[wBishop];
    while (bishops)
    {
      int square = __builtin_ctzll(bishops);
      u64 attacked_squares = mg.get_bishop_attacks(square, board) & zone;
      int count = __builtin_popcountll(attacked_squares);
      eval += count * 2;
      attacker_count = count != 0 ? attacker_count + 1 : attacker_count;
      bishops &= bishops - 1;
    }

    u64 rooks = board.bitboards[wRook];
    while (rooks)
    {
      int square = __builtin_ctzll(rooks);
      u64 attacked_squares = mg.get_rook_attacks(square, board) & zone;
      int count = __builtin_popcountll(attacked_squares);
      eval += count * 3;
      attacker_count = count != 0 ? attacker_count + 1 : attacker_count;
      rooks &= rooks - 1;
    }

    u64 queen = board.bitboards[wQueen];
    while (queen)
    {
      int square = __builtin_ctzll(queen);
      u64 attacked_squares = mg.get_queen_attacks(square, board) & zone;
      int count = __builtin_popcountll(attacked_squares);
      eval += count * 5;
      attacker_count = count != 0 ? attacker_count + 1 : attacker_count;
      queen &= queen - 1;
    }
  }
  double ratio = attacker_count_bonus[std::min(attacker_count, 7)] / 100.0;
  double temp_eval = eval * ratio;
  eval = (int)temp_eval;
  return eval;
}