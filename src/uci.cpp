#include "uci.h"
#include <iostream>
#include <string>
#include <vector>
#include "search.h"
#include "moveGen.h"

void uci_loop(Board &board, MoveGenerator &mg)
{
  std::string cmd;
  std::vector<std::string> split_cmd;
  while (std::getline(std::cin, cmd))
  {
    split_cmd = split_string(cmd, ' ');
    std::string header = split_cmd[0];
    if (header == "uci")
    {
      std::cout << "id name sixseven" << std::endl;
      std::cout << "id author Yichen Xiao" << std::endl;
      std::cout << "uciok" << std::endl;
    }
    else if (header == "isready")
    {
      std::cout << "readyok" << std::endl;
    }
    else if (header == "ucinewgame")
    {
      reset_search();
      board.reset_board();
    }
    else if (header == "position")
    {
      position_handler(split_cmd, board);
    }
    else if (header == "go")
    {
    }
    else if (header == "quit")
    {
      break;
    }
    else
    {
      std::cout << "Invalid command: " << cmd << std::endl;
    }
  }
}

void position_handler(std::vector<std::string> str, Board &board)
{
  if (str.size() < 2) return;
  board.reset_board();
  std::string setup_type = str[1];
  int moves_index = -1;
  for (int i = 2; i < str.size(); i++)
  {
    if (str[i] == "moves")
    {
      moves_index = i;
      break;
    }
  }
  if (setup_type == "fen")
  {
    if (str.size() < 6) return;
    int ply = 0;
    if ((moves_index == -1 || moves_index > 6) && str.size() > 6)
    {
      try { ply = std::stoi(str[6]); } catch (...) { ply = 0; }
    }
    fen_parser(str[2], str[3], str[4], str[5], ply, board);
    if (moves_index != -1)
    {
      for (int i = moves_index + 1; i < str.size(); i++)
      {
        Move move = move_parser(str[i], board);
        board.make_move(move);
      }
    }
  }
  else
  {
    if (moves_index != -1)
    {
      for (int i = moves_index + 1; i < str.size(); i++)
      {
        Move move = move_parser(str[i], board);
        board.make_move(move);
      }
    }
  }
}