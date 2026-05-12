#pragma once
#include <bits/stdc++.h>

void uci_loop();

void go_handler();

void position_handler();

inline std::vector<std::string> split_string(std::string s, char del){
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