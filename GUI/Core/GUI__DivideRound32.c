

#include "GUI_Protected.h"

/*********************************************************************
*
*       GUI__DivideRound32
*
* This routine is used internally for computations. Primary goal is
* to minimize the effects of rounding which occur if we simply
* divide.
*/
I32 GUI__DivideRound32(I32 a, I32 b) {
  if (b) {
    if (a < 0) {
      return (a - (b >> 1)) / b;
    } else {
      return (a + (b >> 1)) / b;
    }
  }
  return 0;
}

/*************************** End of file ****************************/
