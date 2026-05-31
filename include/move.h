#pragma once

#include "types.h"
#include "square.h"

struct Undo
{
  u64 hash;
  u64 pawns_hash;
  int castling_rights;
  int en_passant_square;
  int half_move_count;
  Piece captured_piece;
  bool white_to_move;
  int prev_opening_material;
  int prev_opening_psqt;
  int prev_end_material;
  int prev_end_psqt;
  int prev_phase;
  bool prev_white_castled;
  bool prev_black_castled;
  u64 prev_white_pieces;
  u64 prev_black_pieces;
  u64 prev_occupied_squares;
  Undo() = default;
  Undo(u64 hash, u64 pawns_hash, int castling, int ep, int count, Piece captured, bool white, int op_material, int op_psqt, int ed_material, int ed_psqt, int phase, bool white_castled, bool black_castled) : hash(hash), pawns_hash(pawns_hash), castling_rights(castling), en_passant_square(ep), half_move_count(count), captured_piece(captured), white_to_move(white), prev_opening_material(op_material), prev_opening_psqt(op_psqt), prev_end_material(ed_material), prev_end_psqt(ed_psqt), prev_phase(phase), prev_white_castled(white_castled), prev_black_castled(black_castled){};
};

struct Move
{
  int to;
  int from;
  bool is_castling;
  bool is_kingside;
  bool is_en_passant;
  Piece promotion_piece;
  int set_ep_square;
  Move() : from(0), to(0), set_ep_square(NO_SQUARE), is_castling(false), is_kingside(false), is_en_passant(false), promotion_piece(EMPTY) {};
  Move(int from, int to) : from(from), to(to), set_ep_square(NO_SQUARE), is_castling(false), is_kingside(false), is_en_passant(false), promotion_piece(EMPTY) {};
  Move(int from, int to, Piece promotion) : from(from), to(to), promotion_piece(promotion), is_castling(false), is_kingside(false), is_en_passant(false), set_ep_square(NO_SQUARE) {};
  Move(int from, int to, bool is_ep) : from(from), to(to), is_en_passant(is_ep), is_castling(false), is_kingside(false), set_ep_square(NO_SQUARE), promotion_piece(EMPTY) {};
  Move(int from, int to, int ep_square) : from(from), to(to), set_ep_square(ep_square), is_castling(false), is_kingside(false), is_en_passant(false), promotion_piece(EMPTY) {};

  Move(bool is_castling, bool kingside) : from(0), to(0), is_castling(is_castling), is_kingside(kingside), is_en_passant(false), promotion_piece(EMPTY), set_ep_square(NO_SQUARE) {};
};

struct MoveList
{
  Move moves[256];
  int count = 0;
};
