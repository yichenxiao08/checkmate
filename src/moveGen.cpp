#include "moveGen.h"
#include "board.h"
#include "magics.h"

MoveGenerator::MoveGenerator()
{
  int rook_offset = 0;
  for(int i = 0;i<64;i++){
    rook_attacks[i] = rook_pool + rook_offset;
    rook_offset += (1 << rook_shifts[i]);
  }

  int bishop_offset = 0;
  for(int i = 0;i<64;i++){
    bishop_attacks[i] = bishop_pool + bishop_offset;
    bishop_offset += (1 << bishop_shifts[i]);
  }

  init();
}

void MoveGenerator::generate_moves(Board &board, int ply)
{
  current_ply = ply;
  move_lists[ply].count = 0;
  generate_knight_moves(board);
  generate_king_moves(board);
  generate_pawn_moves(board);
  generate_rook_moves(board);
  generate_bishop_moves(board);
  generate_queen_moves(board);
  if (board.white_to_move)
  {
    if ((board.castling_rights & wK) && !is_attacked(board, true, 4) && !is_attacked(board, true, 5) && !is_attacked(board, true, 6) && board.squares[5] == EMPTY && board.squares[6] == EMPTY)
    {
      add_move(Move(true, true));
    }
    if ((board.castling_rights & wQ) && !is_attacked(board, true, 4) && !is_attacked(board, true, 3) && !is_attacked(board, true, 2) && board.squares[3] == EMPTY && board.squares[2] == EMPTY && board.squares[1] == EMPTY)
    {
      add_move(Move(true, false));
    }
  }
  else
  {
    if ((board.castling_rights & bK) && !is_attacked(board, false, 60) && !is_attacked(board, false, 61) && !is_attacked(board, false, 62) && board.squares[61] == EMPTY && board.squares[62] == EMPTY)
    {
      add_move(Move(true, true));
    }
    if ((board.castling_rights & bQ) && !is_attacked(board, false, 60) && !is_attacked(board, false, 59) && !is_attacked(board, false, 58) && board.squares[59] == EMPTY && board.squares[58] == EMPTY && board.squares[57] == EMPTY)
    {
      add_move(Move(true, false));
    }
  }
}
void MoveGenerator::generate_captures(Board &board, int ply){
  current_ply = ply;
  move_lists[ply].count = 0;
  generate_knight_captures(board);
  generate_king_captures(board);
  generate_pawn_captures(board);
  generate_rook_captures(board);
  generate_bishop_captures(board);
  generate_queen_captures(board);
}
