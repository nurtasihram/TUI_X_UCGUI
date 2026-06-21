

#include "GUI_Protected.h"

/*********************************************************************
*
*       GUI__DivideRound32
*
* This routine is used internally for computations. Primary goal is
* to minimize the effects of rounding which occur if we simply
* divide.
*/
int32_t GUI__DivideRound32(int32_t a, int32_t b) {
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
