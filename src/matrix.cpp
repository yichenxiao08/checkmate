#include "matrix.h"
#include <stdexcept>

float &Matrix::at(int r, int c)
{
  return data[r * cols + c];
}

float Matrix::at(int r, int c) const
{
  return data[r * cols + c];
}

Matrix Matrix::multiply(const Matrix &other) const
{
  int K = this->cols;
  if (K != other.rows)
  {
    throw std::invalid_argument("Incompatible matrix sizes for multiplication");
  }
  Matrix product(this->rows, other.cols);
  for (int i = 0; i < this->rows; i++)
  {
    for (int j = 0; j < other.cols; j++)
    {
      float total = 0;
      for (int k = 0; k < K; k++)
      {
        total += this->at(i, k) * other.at(k, j);
      }
      product.at(i, j) = total;
    }
  }
  return product;
}

Matrix Matrix::transpose() const
{
  Matrix result(this->cols, this->rows);
  for (int i = 0; i < this->rows; i++)
  {
    for (int j = 0; j < this->cols; j++)
    {
      result.at(j, i) = this->at(i, j);
    }
  }
  return result;
}

Matrix Matrix::add(const Matrix &other) const
{
  if (this->rows != other.rows || this->cols != other.cols)
  {
    throw std::invalid_argument("Incompatible matrix sizes for addition");
  }
  Matrix result(other.rows, other.cols);
  for (int i = 0; i < this->rows; i++)
  {
    for (int j = 0; j < this->cols; j++)
    {
      result.at(i, j) = this->at(i, j) + other.at(i, j);
    }
  }
  return result;
}

Matrix Matrix::elementwise_multiply(const Matrix &other) const
{
  if (this->rows != other.rows || this->cols != other.cols)
  {
    throw std::invalid_argument("Incompatible matrix sizes for elementwise multiplication");
  }
  Matrix result(other.rows, other.cols);
  for (int i = 0; i < this->rows; i++)
  {
    for (int j = 0; j < this->cols; j++)
    {
      result.at(i, j) = this->at(i, j) * other.at(i, j);
    }
  }
  return result;
}

Matrix Matrix::apply(float (*func)(float)) const
{
  Matrix result(this->rows, this->cols);
  for (int i = 0; i < this->rows; i++)
  {
    for (int j = 0; j < this->cols; j++)
    {
      result.at(i, j) = func(this->at(i, j));
    }
  }
  return result;
}

Matrix Matrix::scale(float f) const
{
  Matrix result(this->rows, this->cols);
  for (int i = 0; i < this->rows; i++)
  {
    for (int j = 0; j < this->cols; j++)
    {
      result.at(i, j) = this->at(i, j) * f;
    }
  }
  return result;
}

Matrix Matrix::concatenate(const Matrix& x, const Matrix& y)
{
  Matrix result(x.rows + y.rows, 1);
  for (int i = 0; i < x.rows; i++)
  {
    result.at(i, 0) = x.at(i, 0);
  }
  for (int i = 0; i < y.rows; i++)
  {
    result.at(i + x.rows, 0) = y.at(i, 0);
  }
  return result;
}