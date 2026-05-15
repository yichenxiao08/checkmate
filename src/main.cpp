#include "board.h"
#include "moveGen.h"
#include "perft.h"
#include "uci.h"
#include <iostream>

MoveGenerator mg;
Board b;
int main()
{
  uci_loop(b, mg);
  return 0;
}
