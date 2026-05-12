#pragma once
#include "types.h"
#include "move.h"
#include "search_constants.h"

const int table_size = 1 << 20;

struct HashEntry
{
  u64 key;
  int score;
  int depth;
  Move best_move;
  int node_type;
};

extern HashEntry table[table_size];

inline void store_entry(u64 hash, int score, int depth, Move best_move, int node_type, int ply)
{
  HashEntry &entry = table[hash & (table_size - 1)];

  if (score > MATE_THRESHOLD - 10000)
  {
    score += ply;
  }
  else if (score < -MATE_THRESHOLD + 10000)
  {
    score -= ply;
  }

  entry.key = hash;
  entry.score = score;
  entry.depth = depth;
  entry.best_move = best_move;
  entry.node_type = node_type;
}

inline bool probe_entry(u64 hash, int depth, int alpha, int beta, int ply, int &score, Move &m)
{
  HashEntry &entry = table[hash & (table_size - 1)];
  if (entry.key != hash)
    return false;
  m = entry.best_move;
  if (entry.depth >= depth)
  {
    int s = entry.score;
    if (s > MATE_THRESHOLD - 10000)
      s -= ply;
    else if (s < -MATE_THRESHOLD + 10000)
      s += ply;
    if (entry.node_type == EXACT)
    {
      score = s;
      return true;
    }
    if (entry.node_type == UPPER && score <= alpha)
    {
      score = s;
      return true;
    }
    if (entry.node_type == LOWER && score >= beta)
    {
      score = s;
      return true;
    }
  }
  return false;
}
