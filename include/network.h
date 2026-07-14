#pragma once
#include "layer.h"
#include <vector>

class Network
{


public:
  Network(int input_size, int accumulator_size, std::vector<int> remaining_layer_sizes);
  std::vector<Layer> layers;
  int input_size;
  Matrix white_last_input;
  Matrix white_last_z;
  Matrix black_last_input;
  Matrix black_last_z;
  
  Matrix forward(const Matrix& white_input, const Matrix& black_input);
  void backward(const Matrix &target);
  void update_weights(float learning_rate);
};