#include "nnue.h"

NNUE::NNUE() : network(768, 256, {32, 32, 1}), white_accumulator(&network.layers[0], true), black_accumulator(&network.layers[0], false) {}

Matrix NNUE::encode_position(Board &board, bool white)
{
  Matrix pos(768, 1);
  for (int i = 0; i < 12; i++)
  {
    u64 bitboard = board.bitboards[i];
    while (bitboard)
    {
      int square = __builtin_ctzll(bitboard);
      int black_square = square ^ 0b111000;
      int index = white ? i * 64 + square : (i + 6) % 12 * 64 + black_square;
      pos.at(index, 0) = 1.0f;
      bitboard &= bitboard - 1;
    }
  }
  return pos;
}

float NNUE::evaluate(Board &board)
{
  Matrix white_inputs = encode_position(board, true);
  Matrix black_inputs = encode_position(board, false);
}

void Accumulator::refresh(const Matrix &input)
{
  this->accumulator_layer->forward(input);
  this->values = this->accumulator_layer->last_z;
}

void Accumulator::update_features(const Move &m, const Board &board)
{
  int on_pairs[2][2] = {{-1, -1}, {-1, -1}};
  int off_pairs[2][2] = {{-1, -1}, {-1, -1}};
  Piece captured_piece = board.squares[m.to];
  if (m.is_castling)
  {
    if (m.is_kingside && board.is_white_to_move())
    {
      on_pairs[0][0] = wKing;
      on_pairs[0][1] = g1;
      on_pairs[1][0] = wRook;
      on_pairs[1][1] = f1;
      off_pairs[0][0] = wKing;
      off_pairs[0][1] = e1;
      off_pairs[1][0] = wRook;
      off_pairs[1][1] = h1;
    }
    else if (!m.is_kingside && board.is_white_to_move())
    {
      on_pairs[0][0] = wKing;
      on_pairs[0][1] = c1;
      on_pairs[1][0] = wRook;
      on_pairs[1][1] = d1;
      off_pairs[0][0] = wKing;
      off_pairs[0][1] = e1;
      off_pairs[1][0] = wRook;
      off_pairs[1][1] = a1;
    }
    else if (m.is_kingside && !board.is_white_to_move())
    {
      on_pairs[0][0] = bKing;
      on_pairs[0][1] = g8;
      on_pairs[1][0] = bRook;
      on_pairs[1][1] = f8;
      off_pairs[0][0] = bKing;
      off_pairs[0][1] = e8;
      off_pairs[1][0] = bRook;
      off_pairs[1][1] = h8;
    }
    else if (!m.is_kingside && !board.is_white_to_move())
    {
      on_pairs[0][0] = bKing;
      on_pairs[0][1] = c8;
      on_pairs[1][0] = bRook;
      on_pairs[1][1] = d8;
      off_pairs[0][0] = bKing;
      off_pairs[0][1] = e8;
      off_pairs[1][0] = bRook;
      off_pairs[1][1] = a8;
    }
  }
  else if (m.is_en_passant)
  {
    if (board.is_white_to_move())
    {
      on_pairs[0][0] = wPawn;
      on_pairs[0][1] = m.to;
      off_pairs[0][0] = wPawn;
      off_pairs[0][1] = m.from;
      off_pairs[1][0] = bPawn;
      off_pairs[1][1] = m.to - 8;
    }
    else
    {
      on_pairs[0][0] = bPawn;
      on_pairs[0][1] = m.to;
      off_pairs[0][0] = bPawn;
      off_pairs[0][1] = m.from;
      off_pairs[1][0] = wPawn;
      off_pairs[1][1] = m.to + 8;
    }
  }
  else if (captured_piece != EMPTY)
  {
    if (m.promotion_piece != EMPTY)
    {
      if (board.is_white_to_move())
      {
        on_pairs[0][0] = m.promotion_piece;
        on_pairs[0][1] = m.to;
        off_pairs[0][0] = wPawn;
        off_pairs[0][1] = m.from;
        off_pairs[1][0] = captured_piece;
        off_pairs[1][1] = m.to;
      }
      else
      {
        on_pairs[0][0] = m.promotion_piece;
        on_pairs[0][1] = m.to;
        off_pairs[0][0] = bPawn;
        off_pairs[0][1] = m.from;
        off_pairs[1][0] = captured_piece;
        off_pairs[1][1] = m.to;
      }
    }
    else
    {
      on_pairs[0][0] = board.squares[m.from];
      on_pairs[0][1] = m.to;
      off_pairs[0][0] = board.squares[m.from];
      off_pairs[0][1] = m.from;
      off_pairs[1][0] = captured_piece;
      off_pairs[1][1] = m.to;
    }
  }
  else if (m.promotion_piece != EMPTY)
  {
    if (board.is_white_to_move())
    {
      on_pairs[0][0] = m.promotion_piece;
      on_pairs[0][1] = m.to;
      off_pairs[0][0] = wPawn;
      off_pairs[0][1] = m.from;
    }
    else
    {
      on_pairs[0][0] = m.promotion_piece;
      on_pairs[0][1] = m.to;
      off_pairs[0][0] = bPawn;
      off_pairs[0][1] = m.from;
    }
  }
  else
  {
    on_pairs[0][0] = board.squares[m.from];
    on_pairs[0][1] = m.to;
    off_pairs[0][0] = board.squares[m.from];
    off_pairs[0][1] = m.from;
  }

  bool is_white = this->is_white;

  for (int i = 0; i < 2; i++)
  {
    if (on_pairs[i][0] != -1)
    {
      int piece_type = on_pairs[i][0];
      int square = on_pairs[i][1];
      int index = is_white ? piece_type * 64 + square : ((piece_type + 6) % 12) * 64 + (square ^ 0b111000);
      for (int j = 0; j < values.rows; j++)
      {
        values.at(j, 0) += accumulator_layer->weights.at(j, index);
      }
    }
    if (off_pairs[i][0] != -1)
    {
      int piece_type = off_pairs[i][0];
      int square = off_pairs[i][1];
      int index = is_white ? piece_type * 64 + square : ((piece_type + 6) % 12) * 64 + (square ^ 0b111000);
      for (int j = 0; j < values.rows; j++)
      {
        values.at(j, 0) -= accumulator_layer->weights.at(j, index);
      }
    }
  }
}