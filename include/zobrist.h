#pragma once
#include "types.h"

class Board;

extern u64 seed;

inline u64 next()
{
  u64 z = (seed += 0x9e3779b97f4a7c15);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
  z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
  return z ^ (z >> 31);
}

extern u64 square_randoms[12][64];
extern u64 side_key;
extern u64 castling_randoms[16];
extern u64 ep_file[8];

void generate_pseudorandom();

u64 init_hash(Board &b);
