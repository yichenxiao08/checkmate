#include "network.h"
#include <iostream>
#include <cmath>

int test()
{
  Network n({2, 3, 1});
  Matrix input(2, 1);
  input.at(0, 0) = 1.0f;
  input.at(1, 0) = 2.0f;
  Matrix target(1, 1);
  target.at(0, 0) = 0.5f;
  for (int i = 0; i < 500; i++)
  {
    Matrix output = n.forward(input);
    float prediction = output.at(0,0);
    float loss = std::pow((prediction - target.at(0,0)), 2);
    std::cout << "Iteration " << i << ": " << loss << std::endl;
    n.backward(target);
    n.update_weights(0.01f);
  }
  return 0;
}