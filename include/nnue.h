#pragma once
#include "board.h"
#include "network.h"
#include "move.h"

#define INPUT_SIZE 768
#define HL_SIZE 3072

struct Accumulator{
  Matrix values;
  Layer* accumulator_layer;
  bool is_white;
  void update_features(const Move& m, const Board& board);
  void refresh(const Matrix& input);

  Accumulator(Layer* layer, bool white) : accumulator_layer(layer), is_white(white) {};
};

class NNUE
{
public:
  NNUE();
  Network network;
  Accumulator white_accumulator;
  Accumulator black_accumulator;


  Matrix encode_position(Board &board, bool white);
  float evaluate(Board &board);
};