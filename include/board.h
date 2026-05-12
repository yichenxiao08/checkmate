#pragma once

#include "types.h"
#include "square.h"
#include "move.h"

class Board
{

public:
  u64 bitboards[12];
  u64 white_pieces;
  u64 black_pieces;
  u64 occupied_squares;
  bool white_to_move;
  int castling_rights;
  int half_move_count;
  int en_passant_square;
  u64 hash;
  Piece squares[64];

  Board();
  void print_board();
  void make_move(Move &m);
  void update_hash(Move &m);
  void set_color_to_move(bool white)
  {
    white_to_move = white;
  }
  bool is_white_to_move() const { return white_to_move; }
  void unmake_move(Move &m);
  int get_file(int square) { return square % 8; }
  int get_rank(int square) { return square / 8; }
  inline bool is_same_move(Move &a, Move &b)
  {
    if (a.is_castling != b.is_castling)
      return false;
    if (a.is_kingside != b.is_kingside)
      return false;
    if (a.promotion_piece != b.promotion_piece)
      return false;
    if (a.from != b.from)
      return false;
    if (a.to != b.to)
      return false;
    return true;
  }
  void update_position()
  {
    white_pieces = bitboards[wPawn] | bitboards[wKnight] | bitboards[wBishop] | bitboards[wRook] | bitboards[wQueen] | bitboards[wKing];
    black_pieces = bitboards[bPawn] | bitboards[bKnight] | bitboards[bBishop] | bitboards[bRook] | bitboards[bQueen] | bitboards[bKing];
    occupied_squares = white_pieces | black_pieces;
  }
  u64 get_occupied_squares() { return occupied_squares; }
};
