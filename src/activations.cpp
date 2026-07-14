#include "activations.h"

float relu(float x)
{
  return x > 0 ? x : 0.0f;
}

float relu_derivative(float x)
{
  return x > 0 ? 1.0f : 0.0f;
}

float identity(float x)
{
  return x;
}

float identity_derivative(float x)
{
  return 1;
}