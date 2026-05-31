#pragma once
#include "types.h"
#include "move.h"
#include "searchConstants.h"
#include <climits>

const int table_size = 1 << 22;
const int pawn_table_size = 1 << 16;
const int NO_EVAL = INT_MIN;

struct HashEntry
{
  u64 key;
  int score;
  int depth;
  Move best_move;
  int node_type;
  int static_eval;
};

struct PawnEntry
{
  u64 key;
  int score;
};

extern HashEntry table[table_size];
extern PawnEntry pawn_table[pawn_table_size];

void store_entry(u64 hash, int score, int depth, Move best_move, int node_type, int ply, int eval);

bool probe_entry(u64 hash, int depth, int alpha, int beta, int ply, int &score, int &eval, Move &m);

inline void store_pawn_entry(u64 hash, int score)
{
  int index = hash & (pawn_table_size - 1);
  PawnEntry &entry = pawn_table[index];
  entry.key = hash;
  entry.score = score;
}