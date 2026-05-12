#include "evaluation.h"


int material_odds(Board &board)
{
  return piece_vals[wPawn] * (count_pieces(wPawn, board) - count_pieces(bPawn, board))
  + piece_vals[wKnight] * (count_pieces(wKnight, board) - count_pieces(bKnight, board))
  + piece_vals[wBishop] * (count_pieces(wBishop, board) - count_pieces(bBishop, board))
  + piece_vals[wRook] * (count_pieces(wRook, board) - count_pieces(bRook, board))
  + piece_vals[wQueen] * (count_pieces(wQueen, board) - count_pieces(bQueen, board));
}
