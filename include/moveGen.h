#pragma once

#include <vector>
#include "types.h"
#include "move.h"

class Board;

class MoveGenerator
{
public:
  MoveList move_lists[64];
  int current_ply;
  MoveGenerator();
  ~MoveGenerator();
  u64 knight_attack_table[64];
  u64 king_attack_table[64];
  u64 **rook_attack_table;
  u64 **bishop_attack_table;
  u64 rook_masks[64];
  u64 bishop_masks[64];
  u64 white_pawn_attack_table[64];
  u64 black_pawn_attack_table[64];
  void precompute_knight_attacks();
  void precompute_king_attacks();
  void precompute_white_pawn_attacks();
  void precompute_black_pawn_attacks();
  void precompute_rook_masks();
  void precompute_bishop_masks();
  void precompute_rook_attacks();
  void precompute_bishop_attacks();
  u64 calculate_rook_attacks(int sq, u64 blockers);
  u64 calculate_bishop_attacks(int sq, u64 blockers);
  u64 get_rook_attacks(int sq, Board &board);
  u64 get_bishop_attacks(int sq, Board &board);
  u64 get_queen_attacks(int sq, Board &board);
  void init()
  {
    precompute_knight_attacks();
    precompute_king_attacks();
    precompute_white_pawn_attacks();
    precompute_black_pawn_attacks();
    precompute_rook_attacks();
    precompute_bishop_attacks();
  }
  void generate_knight_moves(Board &board);
  void generate_king_moves(Board &board);
  void generate_pawn_moves(Board &board);
  void generate_rook_moves(Board &board);
  void generate_bishop_moves(Board &board);
  void generate_queen_moves(Board &board);
  void add_move(Move m)
  {
    move_lists[current_ply].moves[move_lists[current_ply].count++] = m;
  }
  void generate_moves(Board &board, int ply);
  void generate_knight_captures(Board &board);
  void generate_king_captures(Board &board);
  void generate_pawn_captures(Board &board);
  void generate_rook_captures(Board &board);
  void generate_bishop_captures(Board &board);
  void generate_queen_captures(Board &board);
  void generate_captures(Board &board, int ply);
  bool is_attacked(Board &board, bool white, int sq);
  bool is_in_check(Board &board, bool white);
};
