#include "board.h"
#include "zobrist.h"
#include "search.h"

void Board::unmake_move(Move &m)
{
  repetition_count--;
  hash = m.prev_state.hash;
  castling_rights = m.prev_state.castling_rights;
  en_passant_square = m.prev_state.en_passant_square;
  half_move_count = m.prev_state.half_move_count;
  white_to_move = m.prev_state.white_to_move;
  opening_material = m.prev_state.prev_opening_material;
  opening_psqt = m.prev_state.prev_opening_psqt;
  end_material = m.prev_state.prev_end_material;
  end_psqt = m.prev_state.prev_end_psqt;
  phase = m.prev_state.prev_phase;

  if (m.is_castling)
  {
    if (m.is_kingside && white_to_move)
    {
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << g1)) | (1ULL << e1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << f1)) | (1ULL << h1);
      squares[4] = wKing;
      squares[5] = EMPTY;
      squares[6] = EMPTY;
      squares[7] = wRook;
    }
    else if (!m.is_kingside && white_to_move)
    {
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << c1)) | (1ULL << e1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << d1)) | (1ULL << a1);
      squares[0] = wRook;
      squares[1] = EMPTY;
      squares[2] = EMPTY;
      squares[3] = EMPTY;
      squares[4] = wKing;
    }
    else if (m.is_kingside && !white_to_move)
    {
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << g8)) | (1ULL << e8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << f8)) | (1ULL << h8);
      squares[60] = bKing;
      squares[61] = EMPTY;
      squares[62] = EMPTY;
      squares[63] = bRook;
    }
    else if (!m.is_kingside && !white_to_move)
    {
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << c8)) | (1ULL << e8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << d8)) | (1ULL << a8);
      squares[56] = bRook;
      squares[57] = EMPTY;
      squares[58] = EMPTY;
      squares[59] = EMPTY;
      squares[60] = bKing;
    }
  }
  else if (m.is_en_passant)
  {
    Piece moving_pawn = white_to_move ? wPawn : bPawn;
    Piece captured_pawn = white_to_move ? bPawn : wPawn;
    int captured_sq = white_to_move ? m.to - 8 : m.to + 8;

    bitboards[moving_pawn] = (bitboards[moving_pawn] & ~(1ULL << m.to)) | (1ULL << m.from);
    bitboards[captured_pawn] |= (1ULL << captured_sq);
    squares[m.from] = moving_pawn;
    squares[m.to] = EMPTY;
    squares[captured_sq] = captured_pawn;
  }
  else if (m.promotion_piece != EMPTY)
  {
    Piece pawn_color = white_to_move ? wPawn : bPawn;
    u64 from = 1ULL << m.from;
    u64 to = 1ULL << m.to;
    bitboards[m.promotion_piece] &= ~to;
    bitboards[pawn_color] |= from;
    if (m.prev_state.captured_piece != EMPTY)
    {
      bitboards[m.prev_state.captured_piece] |= to;
    }
    squares[m.to] = m.prev_state.captured_piece;
    squares[m.from] = pawn_color;
  }
  else
  {
    Piece moving_piece = squares[m.to];
    bitboards[moving_piece] = (bitboards[moving_piece] & ~(1ULL << m.to)) | (1ULL << m.from);
    if (m.prev_state.captured_piece != EMPTY)
      bitboards[m.prev_state.captured_piece] |= (1ULL << m.to);
    squares[m.from] = moving_piece;
    squares[m.to] = m.prev_state.captured_piece;
  }

  update_position();
}

void Board::unmake_null_move(Undo &undo_null){
  repetition_count--;
  half_move_count = undo_null.half_move_count;
  white_to_move = !white_to_move;

  en_passant_square = undo_null.en_passant_square;
  castling_rights = undo_null.castling_rights;
  hash = undo_null.hash;
}