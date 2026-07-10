#pragma once
#include "layer.h"
#include <vector>

class Network
{
  private:
  std::vector<Layer> layers;
  int input_size;

  public:
  Network(std::vector<int> layer_sizes);

  Matrix forward(const Matrix& input);
  void backward(const Matrix& target);
  void update_weights(float learning_rate);

};