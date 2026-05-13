#pragma once
#include "types.h"
#include "move.h"
#include "searchConstants.h"
#include <climits>

const int table_size = 1 << 20;
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

extern HashEntry table[table_size];

inline void store_entry(u64 hash, int score, int depth, Move best_move, int node_type, int ply, int eval)
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
  entry.static_eval = eval;
}

inline bool probe_entry(u64 hash, int depth, int alpha, int beta, int ply, int &score, int &eval, Move &m)
{
  HashEntry &entry = table[hash & (table_size - 1)];
  if (entry.key != hash)
    return false;
  m = entry.best_move;
  eval = entry.static_eval;
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
    if (entry.node_type == UPPER && s <= alpha)
    {
      score = s;
      return true;
    }
    if (entry.node_type == LOWER && s >= beta)
    {
      score = s;
      return true;
    }
  }
  return false;
}
