#include "moveGen.h"
#include "board.h"
#include "bitboard_masks.h"

void MoveGenerator::generate_knight_captures(Board &board)
{
  u64 knights = board.white_to_move ? board.bitboards[wKnight] : board.bitboards[bKnight];
  while (knights)
  {
    int knight_pos = __builtin_ctzll(knights);
    u64 opp_pieces = !board.white_to_move ? board.white_pieces : board.black_pieces;
    u64 moves = knight_attack_table[knight_pos] & opp_pieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      add_move(Move(knight_pos, to));
      moves &= moves - 1;
    }
    knights &= knights - 1;
  }
}
void MoveGenerator::generate_king_captures(Board &board)
{
  u64 king = board.white_to_move ? board.bitboards[wKing] : board.bitboards[bKing];
  while (king)
  {
    int king_pos = __builtin_ctzll(king);
    u64 opp_pieces = !board.white_to_move ? board.white_pieces : board.black_pieces;
    u64 moves = king_attack_table[king_pos] & opp_pieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      add_move(Move(king_pos, to));
      moves &= moves - 1;
    }
    king &= king - 1;
  }
}
void MoveGenerator::generate_rook_captures(Board &board)
{
  u64 rook = board.white_to_move ? board.bitboards[wRook] : board.bitboards[bRook];
  while (rook)
  {
    int rook_pos = __builtin_ctzll(rook);
    u64 opp_pieces = !board.white_to_move ? board.white_pieces : board.black_pieces;
    u64 moves = get_rook_attacks(rook_pos, board) & opp_pieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      add_move(Move(rook_pos, to));
      moves &= moves - 1;
    }
    rook &= rook - 1;
  }
}
void MoveGenerator::generate_bishop_captures(Board &board)
{
  u64 bishop = board.white_to_move ? board.bitboards[wBishop] : board.bitboards[bBishop];
  while (bishop)
  {
    int bishop_pos = __builtin_ctzll(bishop);
    u64 opp_pieces = !board.white_to_move ? board.white_pieces : board.black_pieces;
    u64 moves = get_bishop_attacks(bishop_pos, board) & opp_pieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      add_move(Move(bishop_pos, to));
      moves &= moves - 1;
    }
    bishop &= bishop - 1;
  }
}
void MoveGenerator::generate_queen_captures(Board &board)
{
  u64 queen = board.white_to_move ? board.bitboards[wQueen] : board.bitboards[bQueen];
  while (queen)
  {
    int queen_pos = __builtin_ctzll(queen);
    u64 opp_pieces = !board.white_to_move ? board.white_pieces : board.black_pieces;
    u64 moves = get_queen_attacks(queen_pos, board) & opp_pieces;
    while (moves)
    {
      int to = __builtin_ctzll(moves);
      add_move(Move(queen_pos, to));
      moves &= moves - 1;
    }
    queen &= queen - 1;
  }
}
void MoveGenerator::generate_pawn_captures(Board &board)
{
  u64 pawns = board.white_to_move ? board.bitboards[wPawn] : board.bitboards[bPawn];
  while (pawns)
  {
    int pawn_pos = __builtin_ctzll(pawns);
    u64 pawn_pos_bit = 1ULL << pawn_pos;
    bool is_white = board.white_to_move ? true : false;
    u64 opp_pieces = is_white ? board.black_pieces : board.white_pieces;
    u64 captures = is_white ? white_pawn_attack_table[pawn_pos] & opp_pieces : black_pawn_attack_table[pawn_pos] & opp_pieces;
    if (board.en_passant_square != NO_SQUARE)
    {
      u64 ep = 1ULL << board.en_passant_square;
      if (is_white)
      {
        ep &= white_pawn_attack_table[pawn_pos];
      }
      else
      {
        ep &= black_pawn_attack_table[pawn_pos];
      }
      if (ep != 0ULL)
      {
        int to = __builtin_ctzll(ep);
        add_move(Move(pawn_pos, to, true));
      }
    }
    while (captures)
    {
      int to = __builtin_ctzll(captures);
      u64 to_bit = 1ULL << to;
      if (to_bit & promotion_ranks)
      {
        if (is_white)
        {
          add_move(Move(pawn_pos, to, wKnight));
          add_move(Move(pawn_pos, to, wBishop));
          add_move(Move(pawn_pos, to, wRook));
          add_move(Move(pawn_pos, to, wQueen));
        }
        else
        {
          add_move(Move(pawn_pos, to, bKnight));
          add_move(Move(pawn_pos, to, bBishop));
          add_move(Move(pawn_pos, to, bRook));
          add_move(Move(pawn_pos, to, bQueen));
        }
      }
      else
      {
        add_move(Move(pawn_pos, to));
      }
      captures &= captures - 1;
    }
    pawns &= pawns - 1;
  }
}
