#include "layer.h"
#include "activations.h"
#include <random>

Layer::Layer(int input, int output) : input_size(input), output_size(output), weights(output, input), bias(output, 1)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::normal_distribution<float> dist(0.0f, sqrt(2.0 / this->input_size));

  for (int i = 0; i < output; i++)
  {
    for (int j = 0; j < input; j++)
    {
      this->weights.at(i, j) = dist(gen);
    }
  }
}

Matrix Layer::forward(const Matrix &input)
{
  Matrix z = weights.multiply(input).add(bias);
  Matrix output = z.apply(relu);

  this->last_z = z;
  this->last_input = input;
  this->last_output = output;

  return output;
}

Matrix Layer::backward(const Matrix &gradient)
{
  Matrix relu_mask = last_z.apply(relu_derivative);
  Matrix dL_dz = gradient.elementwise_multiply(relu_mask);
  Matrix grad_weights = dL_dz.multiply(last_input.transpose());
  this->grad_weights = grad_weights;
  this->grad_bias = dL_dz;
  Matrix grad_inputs = this->weights.transpose().multiply(dL_dz);
  return grad_inputs;
}

void Layer::update_weights(float learning_rate)
{
  this->weights = this->weights.add(this->grad_weights.scale(-learning_rate));
  this->bias = this->bias.add(this->grad_bias.scale(-learning_rate));
}
