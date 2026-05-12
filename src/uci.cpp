#include "uci.h"
#include <iostream>
#include <string>
#include <vector>

void loop()
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
    }
    else if (header == "position")
    {
    } else if (header == "go"){

    }else if (header == "stop"){

    } else {
      std::cout << "Invalid command: " << cmd << std::endl;
    }
  }
}