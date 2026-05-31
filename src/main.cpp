#include "board.h"
#include "moveGen.h"
#include "uci.h"
#include <iostream>
#include "evaluation.h"
#include "move.h"
#include "perft.h"
MoveGenerator mg;
Board b;
int main()
{
  uci_loop(b, mg);
  // std::cout << perft(b, 6, true, mg);
  // Move m1(12, 20);
  // b.make_move(m1);

  // Move m2(62, 45);
  // b.make_move(m2);

  // std::cout << evaluate_king_safety(b, 24, mg) << std::endl;

  return 0;
}
