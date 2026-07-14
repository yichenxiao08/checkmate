#include "board.h"
#include "moveGen.h"
#include "uci.h"
#include <iostream>
#include "evaluation.h"
#include "move.h"
#include "perft.h"
#include "network.h"
#include <iostream>
#include <cmath>
#include <random>
void test()
{
  std::random_device rd;

  // 2. Initialize the standard mersenne twister engine with the seed
  std::mt19937 gen(rd());

  // 3. Define the inclusive range [min, max]
  std::uniform_int_distribution<int> distr(0, 5);

  Network n(4, 3, {2, 1});
  Matrix white_inputs[5];
  Matrix black_inputs[5];
  Matrix outputs[5];
  for (int i = 0; i < 5; i++)
  {
    Matrix white_input(4, 1);
    Matrix black_input(4, 1);
    for(int i = 0;i<4;i++){
      white_input.at(i, 0) = distr(gen);
      black_input.at(i, 0) = distr(gen);
    }
    Matrix target(1, 1);
    target.at(0, 0) = distr(gen);
    white_inputs[i] = white_input;
    black_inputs[i] = black_input;
    outputs[i] = target;
  }
  for (int i = 0; i < 200; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      Matrix output = n.forward(white_inputs[j], black_inputs[j]);
      float prediction = output.at(0, 0);
      float loss = std::pow((prediction - outputs[j].at(0, 0)), 2);
      // std::cout << "Iteration " << i << ", sample " << j << ": " << loss << std::endl;
      n.backward(outputs[j]);
      n.update_weights(0.01f);
      std::cout << n.layers[0].weights.at(0, 0) << std::endl;
    }
  }
}

MoveGenerator mg;
Board b;
int main()
{
  test();
  // uci_loop(b, mg);

  return 0;
}
