#include "network.h"

Network::Network(std::vector<int> layer_sizes)
{
  this->input_size = layer_sizes[0];
  int last_input_size = layer_sizes[0];
  for (int i = 1; i < layer_sizes.size(); i++)
  {
    this->layers.push_back(Layer(last_input_size, layer_sizes[i]));
    last_input_size = layer_sizes[i];
  }
}

Matrix Network::forward(const Matrix &input)
{
  Matrix next = input;
  for (Layer &layer : this->layers)
  {
    next = layer.forward(next);
  }
  return next;
}

void Network::backward(const Matrix &target)
{
  Matrix output = layers.back().last_output;
  Matrix gradient = output.add(target.scale(-1.0f)).scale(2);
  for (int i = layers.size() - 1; i >= 0; i--)
  {
    gradient = layers[i].backward(gradient);
  }
}

void Network::update_weights(float learning_rate){
  for(Layer& layer: layers){
    layer.update_weights(learning_rate);
  }
}