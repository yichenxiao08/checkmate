#pragma once
#include <bits/stdc++.h>
#include <cstdlib>
#include "square.h"
#include "types.h"
#include "board.h"
#include "moveGen.h"

void uci_loop(Board &board, MoveGenerator &mg);

void go_handler(std::vector<std::string> str);

void position_handler(std::vector<std::string> str, Board &board);

inline std::vector<std::string> split_string(std::string s, char del)
{
  std::vector<std::string> split;
  std::stringstream ss(s);
  std::string word;
  while (!ss.eof())
  {
    std::getline(ss, word, del);
    split.push_back(word);
  }
  return split;
}

inline Square parse_square(std::string sq_str)
{
  if (sq_str == "-")
    return NO_SQUARE;
  int file = sq_str[0] - 'a';
  int rank = sq_str[1] - '1';
  return (Square)(file + rank * 8);
}

inline int parse_castling(std::string castling)
{
  int castling_rights = 0;
  if (castling.find("K") != std::string::npos)
    castling_rights |= wK;
  if (castling.find("Q") != std::string::npos)
    castling_rights |= wQ;
  if (castling.find("k") != std::string::npos)
    castling_rights |= bK;
  if (castling.find("q") != std::string::npos)
    castling_rights |= bQ;
  return castling_rights;
}

inline Piece character_to_piece(char c)
{
  switch (c)
  {
  case 'P':
    return wPawn;
  case 'N':
    return wKnight;
  case 'B':
    return wBishop;
  case 'R':
    return wRook;
  case 'Q':
    return wQueen;
  case 'K':
    return wKing;
  case 'p':
    return bPawn;
  case 'n':
    return bKnight;
  case 'b':
    return bBishop;
  case 'r':
    return bRook;
  case 'q':
    return bQueen;
  case 'k':
    return bKing;
  default:
    return EMPTY;
  }
}

inline void fen_parser(std::string fen, std::string white_to_move, std::string castling, std::string en_passant, int ply, Board &board)
{
  board.white_to_move = white_to_move == "w" ? true : false;
  board.en_passant_square = parse_square(en_passant);
  board.castling_rights = parse_castling(castling);
  board.half_move_count = ply;
  int rank = 7;
  int index = 0;
  for (int i = 0; i < fen.length(); i++)
  {
    char c = fen.at(i);
    if (c == '/')
    {
      rank--;
      index = 0;
      continue;
    }
    else if (c >= '0' && c <= '9')
    {
      for (int j = 0; j < (c - '0'); j++)
      {
        board.squares[rank * 8 + index++] = EMPTY;
      }
    }
    else
    {
      Piece p = character_to_piece(c);
      board.squares[rank * 8 + index++] = p;
    }
  }
  for (int i = 0; i < 12; i++)
  {
    board.bitboards[i] = 0ULL;
  }

  for (int i = 0; i < 64; i++)
  {
    if (board.squares[i] != EMPTY)
    {
      board.bitboards[board.squares[i]] |= (1ULL << i);
    }
  }
  board.update_position();
}

inline Move move_parser(std::string move, Board &board){
  if(move == "e1g1" && board.squares[e1] == wKing) return Move(true, true);
  if(move == "e8g8" && board.squares[e8] == bKing) return Move(true, true);
  if(move == "e1c1" && board.squares[e1] == wKing) return Move(true, false);
  if(move == "e8c8" && board.squares[e8] == bKing) return Move(true, false);
  Square from = parse_square(move.substr(0, 2));
  char from_file = move.at(0);
  char from_rank = move.at(1);
  Square to = parse_square(move.substr(2, 2));
  char to_file = move.at(2);
  char to_rank = move.at(3);
  if(move.length() > 4) {
    char promotion = move.at(4);
    if(promotion == 'n') return board.is_white_to_move() ? Move(from, to, wKnight) : Move(from, to, bKnight);
    if(promotion == 'b') return board.is_white_to_move() ? Move(from, to, wBishop) : Move(from, to, bBishop);
    if(promotion == 'r') return board.is_white_to_move() ? Move(from, to, wRook) : Move(from, to, bRook);
    if(promotion == 'q') return board.is_white_to_move() ? Move(from, to, wQueen) : Move(from, to, bQueen);
  }
  if(from_file != to_file && board.squares[from] % 6 == 0 && board.squares[to] == EMPTY){
    return Move(from, to, true);
  }
  if(board.squares[from] % 6 == 0 && std::abs(from_rank - to_rank) == 2) {
    return board.is_white_to_move() ? Move(from, to, to - 8) : Move(from, to, to + 8);
  }
  return Move(from, to);
}