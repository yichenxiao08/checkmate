#include "network.h"
#include "activations.h"
Network::Network(int input_size, int accumulator_size, std::vector<int> remaining_layer_sizes)
{
  this->input_size = input_size;
  this->layers.push_back(Layer(input_size, accumulator_size, relu, relu_derivative));
  int last_input_size = accumulator_size * 2;
  for (int i = 0; i < remaining_layer_sizes.size(); i++)
  {
    float (*activation)(float) = i == remaining_layer_sizes.size() - 1 ? identity : relu;
    float (*activation_derivative)(float) = i == remaining_layer_sizes.size() - 1 ? identity_derivative : relu_derivative;
    this->layers.push_back(Layer(last_input_size, remaining_layer_sizes[i], activation, activation_derivative));
    last_input_size = remaining_layer_sizes[i];
  }
}

Matrix Network::forward(const Matrix &white_input, const Matrix &black_input)
{
  Matrix white_acc = layers[0].forward(white_input);
  this->white_last_input = layers[0].last_input;
  this->white_last_z = layers[0].last_z;

  Matrix black_acc = layers[0].forward(black_input);
  this->black_last_input = layers[0].last_input;
  this->black_last_z = layers[0].last_z;

  Matrix next = Matrix::concatenate(white_acc, black_acc);

  for (int i = 1; i < layers.size(); i++)
  {
    next = layers[i].forward(next);
  }
  return next;
}

void Network::backward(const Matrix &target)
{
  Matrix output = layers.back().last_output;
  Matrix gradient = output.add(target.scale(-1.0f)).scale(2);
  for (int i = layers.size() - 1; i >= 1; i--)
  {
    gradient = layers[i].backward(gradient);
  }

  int half_size = gradient.rows / 2;
  Matrix white_gradient(half_size, 1);
  Matrix black_gradient(half_size, 1);
  for (int i = 0; i < half_size; i++)
  {
    white_gradient.at(i, 0) = gradient.at(i, 0);
    black_gradient.at(i, 0) = gradient.at(i + half_size, 0);
  }

  layers[0].last_input = white_last_input;
  layers[0].last_z = white_last_z;
  layers[0].backward(white_gradient);

  Matrix white_grad_weights = layers[0].grad_weights;
  Matrix white_grad_bias = layers[0].grad_bias;

  layers[0].last_input = black_last_input;
  layers[0].last_z = black_last_z;
  layers[0].backward(black_gradient);

  layers[0].grad_weights = layers[0].grad_weights.add(white_grad_weights);
  layers[0].grad_bias = layers[0].grad_bias.add(white_grad_bias);
}

void Network::update_weights(float learning_rate)
{
  for (Layer &layer : layers)
  {
    layer.update_weights(learning_rate);
  }
}