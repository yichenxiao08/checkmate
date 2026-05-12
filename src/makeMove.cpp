#include "board.h"
#include "castling.h"
#include "zobrist.h"

void Board::make_move(Move &m)
{
  m.prev_state = Undo{hash, castling_rights, en_passant_square, half_move_count, squares[m.to], white_to_move};

  hash ^= castling_randoms[castling_rights];
  if (en_passant_square != NO_SQUARE)
    hash ^= ep_file[en_passant_square % 8];

  if (m.is_castling)
  {
    if (m.is_kingside && white_to_move)
    {
      hash ^= square_randoms[wKing][e1];
      hash ^= square_randoms[wRook][h1];
      castling_rights &= ~(wK | wQ);
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << e1)) | (1ULL << g1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << h1)) | (1ULL << f1);
      squares[4] = EMPTY;
      squares[5] = wRook;
      squares[6] = wKing;
      squares[7] = EMPTY;
      hash ^= square_randoms[wKing][g1];
      hash ^= square_randoms[wRook][f1];
    }
    else if (!m.is_kingside && white_to_move)
    {
      hash ^= square_randoms[wKing][e1];
      hash ^= square_randoms[wRook][a1];
      castling_rights &= ~(wK | wQ);
      bitboards[wKing] = (bitboards[wKing] & ~(1ULL << e1)) | (1ULL << c1);
      bitboards[wRook] = (bitboards[wRook] & ~(1ULL << a1)) | (1ULL << d1);
      squares[0] = EMPTY;
      squares[1] = EMPTY;
      squares[2] = wKing;
      squares[3] = wRook;
      squares[4] = EMPTY;
      hash ^= square_randoms[wKing][c1];
      hash ^= square_randoms[wRook][d1];
    }
    else if (m.is_kingside && !white_to_move)
    {
      hash ^= square_randoms[bKing][e8];
      hash ^= square_randoms[bRook][h8];
      castling_rights &= ~(bK | bQ);
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << e8)) | (1ULL << g8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << h8)) | (1ULL << f8);
      squares[60] = EMPTY;
      squares[61] = bRook;
      squares[62] = bKing;
      squares[63] = EMPTY;
      hash ^= square_randoms[bKing][g8];
      hash ^= square_randoms[bRook][f8];
    }
    else if (!m.is_kingside && !white_to_move)
    {
      hash ^= square_randoms[bKing][e8];
      hash ^= square_randoms[bRook][a8];
      castling_rights &= ~(bK | bQ);
      bitboards[bKing] = (bitboards[bKing] & ~(1ULL << e8)) | (1ULL << c8);
      bitboards[bRook] = (bitboards[bRook] & ~(1ULL << a8)) | (1ULL << d8);
      squares[56] = EMPTY;
      squares[57] = EMPTY;
      squares[58] = bKing;
      squares[59] = bRook;
      squares[60] = EMPTY;
      hash ^= square_randoms[bKing][c8];
      hash ^= square_randoms[bRook][d8];
    }
  }
  else if (m.is_en_passant)
  {
    m.prev_state.captured_piece = white_to_move ? bPawn : wPawn;
    Piece moving_pawn = white_to_move ? wPawn : bPawn;
    Piece captured_pawn = white_to_move ? bPawn : wPawn;
    int captured_sq = white_to_move ? m.to - 8 : m.to + 8;

    hash ^= square_randoms[moving_pawn][m.from];
    hash ^= square_randoms[captured_pawn][captured_sq];

    u64 from = 1ULL << m.from;
    u64 to = 1ULL << m.to;
    u64 captured_square = white_to_move ? 1ULL << (m.to - 8) : 1ULL << (m.to + 8);
    squares[m.to] = squares[m.from];
    squares[m.from] = EMPTY;
    if (white_to_move)
    {
      bitboards[wPawn] &= ~from;
      bitboards[wPawn] |= to;
      bitboards[bPawn] &= ~captured_square;
      squares[m.to - 8] = EMPTY;
    }
    else
    {
      bitboards[bPawn] &= ~from;
      bitboards[bPawn] |= to;
      bitboards[wPawn] &= ~captured_square;
      squares[m.to + 8] = EMPTY;
    }

    hash ^= square_randoms[moving_pawn][m.to];
  }
  else if (m.promotion_piece != EMPTY)
  {
    Piece pawn_color = white_to_move ? wPawn : bPawn;
    Piece piece_from = squares[m.from];
    Piece piece_to = squares[m.to];
    u64 from = 1ULL << m.from;
    u64 to = 1ull << m.to;

    hash ^= square_randoms[pawn_color][m.from];
    if (piece_to != EMPTY)
      hash ^= square_randoms[piece_to][m.to];

    squares[m.to] = m.promotion_piece;
    squares[m.from] = EMPTY;
    bitboards[m.promotion_piece] |= to;
    bitboards[pawn_color] &= ~from;
    if (piece_to != EMPTY)
      bitboards[piece_to] &= ~to;
    castling_rights &= castling_rights_mask[m.to];

    hash ^= square_randoms[m.promotion_piece][m.to];
  }
  else
  {
    Piece piece_from = squares[m.from];
    Piece piece_to = squares[m.to];
    u64 from = 1ULL << m.from;
    u64 to = 1ULL << m.to;

    hash ^= square_randoms[piece_from][m.from];
    if (piece_to != EMPTY)
      hash ^= square_randoms[piece_to][m.to];

    squares[m.to] = squares[m.from];
    squares[m.from] = EMPTY;
    bitboards[piece_from] &= ~from;
    bitboards[piece_from] |= to;
    if (piece_to != EMPTY)
      bitboards[piece_to] &= ~to;
    castling_rights &= castling_rights_mask[m.from];
    castling_rights &= castling_rights_mask[m.to];

    hash ^= square_randoms[piece_from][m.to];
  }

  half_move_count++;
  white_to_move = !white_to_move;
  en_passant_square = m.set_ep_square;

  hash ^= castling_randoms[castling_rights];
  if (en_passant_square != NO_SQUARE)
    hash ^= ep_file[en_passant_square % 8];

  hash ^= side_key;

  update_position();
}
