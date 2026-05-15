#pragma once

#include "types.h"
#include "square.h"
#include "move.h"
#include "evalConstants.h"
#include "zobrist.h"

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

  int opening_material;
  int end_material;
  int opening_psqt;
  int end_psqt;
  int phase;

  Board();
  void print_board();

  void make_move(Move &m);
  void make_null_move(Undo &undo_null);
  void unmake_move(Move &m);
  void unmake_null_move(Undo &undo_null);

  inline void add_piece_eval(Piece p, int sq)
  {
    bool isWhite = p < 6;
    int sign = isWhite ? 1 : -1;
    int square = isWhite ? sq : (sq ^ 56);
    opening_material += opening_piece_vals[p] * sign;
    end_material += end_piece_vals[p] * sign;
    opening_psqt += opening_tables[p % 6][square] * sign;
    end_psqt += end_tables[p % 6][square] * sign;
    phase += phase_weights[p];
  }

  inline void remove_piece_eval(Piece p, int sq)
  {
    bool isWhite = p < 6;
    int sign = isWhite ? 1 : -1;
    int square = isWhite ? sq : (sq ^ 56);
    opening_material -= opening_piece_vals[p] * sign;
    end_material -= end_piece_vals[p] * sign;
    opening_psqt -= opening_tables[p % 6][square] * sign;
    end_psqt -= end_tables[p % 6][square] * sign;
    phase -= phase_weights[p];
  }

  inline bool is_white_to_move() const { return white_to_move; }
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
  void reset_board();
  inline void update_position()
  {
    white_pieces = bitboards[wPawn] | bitboards[wKnight] | bitboards[wBishop] | bitboards[wRook] | bitboards[wQueen] | bitboards[wKing];
    black_pieces = bitboards[bPawn] | bitboards[bKnight] | bitboards[bBishop] | bitboards[bRook] | bitboards[bQueen] | bitboards[bKing];
    occupied_squares = white_pieces | black_pieces;
    hash = init_hash(*this);
  }
  inline bool has_piece_material()
  {
    return bitboards[wKnight] | bitboards[wBishop] | bitboards[wRook] | bitboards[wQueen] | bitboards[bKnight] | bitboards[bBishop] | bitboards[bRook] | bitboards[bQueen];
  }
};
