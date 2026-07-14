#include "matrix.h"
#pragma once

struct Layer {
  int input_size;
  int output_size;

  float (*activation)(float);
  float (*activation_derivative)(float);

  Matrix weights;
  Matrix bias;

  Matrix last_input;
  Matrix last_z;
  Matrix last_output;
  
  Matrix grad_weights;
  Matrix grad_bias;

  Layer(int input, int output, float (*activation)(float), float (*activation_derivative)(float));
  Matrix forward(const Matrix& input);
  Matrix backward(const Matrix& gradient);

  void update_weights(float learning_rate);

  void gradient_check();

};
