#include "board.h"
#include "evaluation.h"
#include <iostream>
#include <assert.h>

void Board::reset_board(){
  castling_rights = 15;
  half_move_count = 0;
  Piece initial[64] = {
      // Rank 1
      wRook, wKnight, wBishop, wQueen, wKing, wBishop, wKnight, wRook,

      // Rank 2
      wPawn, wPawn, wPawn, wPawn, wPawn, wPawn, wPawn, wPawn,

      // Rank 3
      EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,

      // Rank 4
      EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,

      // Rank 5
      EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,

      // Rank 6
      EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,

      // Rank 7
      bPawn, bPawn, bPawn, bPawn, bPawn, bPawn, bPawn, bPawn,

      // Rank 8
      bRook, bKnight, bBishop, bQueen, bKing, bBishop, bKnight, bRook};

  for (int i = 0; i < 64; i++)
    squares[i] = initial[i];

  for (int i = 0; i < 12; i++)
  {
    bitboards[i] = 0ULL;
  }

  for (int i = 0; i < 64; i++)
  {
    if (squares[i] != EMPTY)
    {
      bitboards[squares[i]] |= (1ULL << i);
    }
  }
  update_position();
  white_to_move = true;
  castling_rights = wK | wQ | bK | bQ;
  half_move_count = 0;
  en_passant_square = NO_SQUARE;

  opening_material = 0;
  end_material = 0;
  opening_psqt = 0;
  end_psqt = 0;
  phase = 0;
  precompute_eval(*this);
}

Board::Board()
{
  reset_board();
}

void Board::print_board()
{
  for (int i = 7; i >= 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      switch (squares[i * 8 + j])
      {
      case EMPTY:
        std::cout << ". ";
        break;
      case wRook:
        std::cout << "R ";
        break;
      case bRook:
        std::cout << "r ";
        break;
      case wKnight:
        std::cout << "N ";
        break;
      case bKnight:
        std::cout << "n ";
        break;
      case wBishop:
        std::cout << "B ";
        break;
      case bBishop:
        std::cout << "b ";
        break;
      case wQueen:
        std::cout << "Q ";
        break;
      case bQueen:
        std::cout << "q ";
        break;
      case wKing:
        std::cout << "K ";
        break;
      case bKing:
        std::cout << "k ";
        break;
      case wPawn:
        std::cout << "P ";
        break;
      case bPawn:
        std::cout << "p ";
        break;
      }
    }
    std::cout << "\n";
  }
}
