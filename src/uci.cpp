#include "uci.h"
#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "search.h"
#include "moveGen.h"
#include "perft.h"

void uci_loop(Board &board, MoveGenerator &mg)
{
  std::atomic<bool> stop_flag = false;
  std::thread search_thread;

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
      repetition_count = 0;
    }
    else if (header == "position")
    {
      position_handler(split_cmd, board);
    }
    else if (header == "go")
    {
      if (search_thread.joinable())
      {
        stop_flag = true;
        search_thread.join();
      }
      stop_flag = false;
      go_handler(split_cmd, stop_flag, search_thread, board, mg);
    }
    else if (header == "quit")
    {
      stop_flag = true;
      if (search_thread.joinable())
      {
        search_thread.join();
      }
      break;
    }
    else if (header == "stop")
    {
      stop_flag = true;
      if (search_thread.joinable())
      {
        search_thread.join();
      }
    }
    else
    {
      std::cout << "Invalid command: " << cmd << std::endl;
    }
  }
}

void position_handler(std::vector<std::string> str, Board &board)
{
  if (str.size() < 2)
    return;
  board.reset_board();
  repetition_count = 0;
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
    if (str.size() < 6)
      return;
    int ply = 0;
    if ((moves_index == -1 || moves_index > 6) && str.size() > 6)
    {
      try
      {
        ply = std::stoi(str[6]);
      }
      catch (...)
      {
        ply = 0;
      }
    }
    fen_parser(str[2], str[3], str[4], str[5], ply, board);
    repetition_table[repetition_count++] = board.hash;
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
    repetition_table[repetition_count++] = board.hash;
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

void go_handler(std::vector<std::string> str, std::atomic<bool> &stop_flag, std::thread &search, Board &board, MoveGenerator &mg)
{
  int depth = 64;

  int wtime = 0;
  int btime = 0;
  int winc = 0;
  int binc = 0;

  int player_time;
  int opp_time;
  int player_inc;
  int opp_inc;

  for (int i = 1; i + 1 < str.size(); i++)
  {
    if (str[i] == "depth")
    {
      try
      {
        depth = std::stoi(str[i + 1]);
      }
      catch (...)
      {
      }
    }
    else if (str[i] == "wtime")
    {
      try
      {
        wtime = std::stoi(str[i + 1]);
      }
      catch (...)
      {
      }
    }
    else if (str[i] == "btime")
    {
      try
      {
        btime = std::stoi(str[i + 1]);
      }
      catch (...)
      {
      }
    }
    else if (str[i] == "winc")
    {
      try
      {
        winc = std::stoi(str[i + 1]);
      }
      catch (...)
      {
      }
    }
    else if (str[i] == "binc")
    {
      try
      {
        binc = std::stoi(str[i + 1]);
      }
      catch (...)
      {
      }
    }
  }

  if (board.is_white_to_move())
  {
    player_time = wtime;
    opp_time = btime;
    player_inc = winc;
    opp_inc = binc;
  }
  else
  {
    player_time = btime;
    opp_time = wtime;
    player_inc = binc;
    opp_inc = winc;
  }

  int time_allotted = player_time / 30 + player_inc / 2;
  auto start = std::chrono::steady_clock::now();
  search = std::thread([&, depth, start, time_allotted]()
                       {
    std::thread timer([&, time_allotted]() {
      auto deadline = std::chrono::steady_clock::now()
                    + std::chrono::milliseconds(time_allotted);
      while (!stop_flag.load(std::memory_order_relaxed) &&
             std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
      }
      stop_flag.store(true);
    });

    int depth_searched = 0;
    Move best = iterative_deepening(board, mg, depth, stop_flag, depth_searched, start, time_allotted);
    stop_flag.store(true);
    timer.join();
    std::cout << "bestmove " << move_to_string(best, board) << "\n";
    std::cout << "depth searched " << depth_searched << "\n"; });
}