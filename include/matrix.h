#pragma once
#include <vector>
struct Matrix
{
  int rows, cols;
  std::vector<float> data;
  float &at(int r, int c);
  float at(int r, int c) const;
  Matrix() : rows(0), cols(0) {}
  Matrix(int r, int c): rows(r), cols(c), data(r*c, 0.0f) {}
  Matrix multiply(const Matrix &other) const;
  Matrix transpose() const;
  Matrix add(const Matrix &other) const;
  Matrix elementwise_multiply(const Matrix &other) const;
  Matrix apply(float (*func)(float)) const;
  Matrix scale(float f) const;
  static Matrix concatenate(const Matrix& x, const Matrix& y) ;
};