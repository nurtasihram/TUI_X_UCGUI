

#include "GUI_Protected.h"

/*********************************************************************
*
*       GUI__DivideRound
*
* This routine is used internally for computations. Primary goal is
* to minimize the effects of rounding which occur if we simply
* divide.
*/
int GUI__DivideRound(int a, int b) {
  int r = 0;
  if (b) {
    r = ((a + b / 2) / b);
  }
  return r;
}


