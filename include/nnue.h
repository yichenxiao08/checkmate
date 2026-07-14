#pragma once
#include "board.h"
#include "network.h"

#define INPUT_SIZE 768
#define HL_SIZE 3072

class NNUE
{
public:
  NNUE();
  Network network;
  Matrix encode_position(Board &board, bool white);
  void evaluate(Board &board);
};
