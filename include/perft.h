#pragma once

#include "moveGen.h"
#include "board.h"
#include "evaluation.h"
#include "string"
#include <iostream>
#include <assert.h>
std::string move_to_string(Move &m, Board &board)
{
  if (m.is_castling)
  {
    if (m.is_kingside)
      if (board.is_white_to_move())
        return "e1g1";
      else
        return "e8g8";
    else
    {
      if (board.is_white_to_move())
        return "e1c1";
      else
        return "e8c8";
    }
  }
  std::string result = "";
  result += (char)('a' + (m.from % 8));
  result += (char)('1' + (m.from / 8));
  result += (char)('a' + (m.to % 8));
  result += (char)('1' + (m.to / 8));
  if(m.promotion_piece != EMPTY){
    if(m.promotion_piece % 6 == 1) result += 'n';
    if(m.promotion_piece % 6 == 2) result += 'b';
    if(m.promotion_piece % 6 == 3) result += 'r';
    if(m.promotion_piece % 6 == 4) result += 'q';
  }
  return result;
}

u64 perft(Board &board, int depth, bool white, MoveGenerator &move_gen, int ply = 0)
{
  if (depth == 0)
    return 1;

  move_gen.generate_moves(board, ply);
  int count = move_gen.move_lists[ply].count;

  u64 nodes = 0;
  for (int i = 0; i < count; i++)
  {
    Move m = move_gen.move_lists[ply].moves[i];
    board.make_move(m);
    if (!move_gen.is_in_check(board, white))
    {
      int current_nodes = perft(board, depth - 1, !white, move_gen, ply + 1);
      // if (ply == 0)
      //   std::cout << "\n"
      //             << move_to_string(m, board) << " " << current_nodes;
      nodes += current_nodes;
      
    }
    board.unmake_move(m);
  }
  return nodes;
}


