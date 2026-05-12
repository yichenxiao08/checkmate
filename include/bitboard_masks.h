#pragma once
#include "types.h"

const u64 not_a_file = 0xFEFEFEFEFEFEFEFE;
const u64 not_h_file = 0x7F7F7F7F7F7F7F7F;
const u64 not_ab_file = 0xFCFCFCFCFCFCFCFC;
const u64 not_gh_file = 0x3F3F3F3F3F3F3F3F;
const u64 second_rank = 0x000000000000FF00;
const u64 seventh_rank = 0x00FF000000000000;
const u64 first_rank = 0x00000000000000FF;
const u64 eighth_rank = 0xFF00000000000000;
const u64 board_edges = 0xFF818181818181FF;
const u64 not_edges = ~board_edges;
const u64 promotion_ranks = 0xFF000000000000FF;
