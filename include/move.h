#pragma once

#include "types.h"

struct Undo
{
  u64 hash;
  int castling_rights;
  int en_passant_square;
  int half_move_count;
  Piece captured_piece;
  bool white_to_move;
  Undo() = default;
  Undo(u64 hash, int castling, int ep, int count, Piece captured, bool white) : hash(hash), castling_rights(castling), en_passant_square(ep), half_move_count(count), captured_piece(captured), white_to_move(white) {};
};

struct Move
{
  int to;
  int from;
  bool is_castling;
  bool is_kingside;
  bool is_en_passant;
  Piece promotion_piece;
  Undo prev_state;
  int set_ep_square;
  Move() : from(0), to(0), set_ep_square(-1), is_castling(false), is_kingside(false), is_en_passant(false), promotion_piece(EMPTY) {};
  Move(int from, int to) : from(from), to(to), set_ep_square(-1), is_castling(false), is_kingside(false), is_en_passant(false), promotion_piece(EMPTY) {};
  Move(int from, int to, Piece promotion) : from(from), to(to), promotion_piece(promotion), is_castling(false), is_en_passant(false), set_ep_square(-1) {};
  Move(int from, int to, bool is_ep) : from(from), to(to), is_en_passant(is_ep), is_castling(false), set_ep_square(-1), promotion_piece(EMPTY) {};
  Move(int from, int to, int ep_square) : from(from), to(to), set_ep_square(ep_square), is_castling(false), is_en_passant(false), promotion_piece(EMPTY) {};

  Move(bool is_castling, bool kingside) : from(0), to(0), is_castling(is_castling), is_kingside(kingside), is_en_passant(false), promotion_piece(EMPTY), set_ep_square(-1) {};
};

struct MoveList
{
  Move moves[218];
  int count = 0;
};
