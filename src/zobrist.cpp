#include "zobrist.h"
#include "board.h"

u64 seed = 67;

u64 square_randoms[12][64];
u64 side_key;
u64 castling_randoms[16];
u64 ep_file[8];

void generate_pseudorandom()
{
  for (int i = 0; i < 12; i++)
  {
    for (int j = 0; j < 64; j++)
    {
      square_randoms[i][j] = next();
    }
  }
  side_key = next();
  for (int i = 0; i < 16; i++)
  {
    castling_randoms[i] = next();
  }
  for (int i = 0; i < 8; i++)
  {
    ep_file[i] = next();
  }
}

u64 init_hash(Board &b)
{
  static bool initialized = false;
  if (!initialized)
  {
    generate_pseudorandom();
    initialized = true;
  }
  u64 hash = 0;
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      int index = 8 * i + j;
      if (b.squares[index] != EMPTY)
      {
        Piece p = b.squares[index];
        hash ^= square_randoms[p][index];
      }
    }
  }
  if (!b.white_to_move)
    hash ^= side_key;

  hash ^= castling_randoms[b.castling_rights];
  if (b.en_passant_square != NO_SQUARE)
  {
    int file = b.en_passant_square % 8;
    hash ^= ep_file[file];
  }
  return hash;
}
