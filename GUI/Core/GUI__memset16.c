

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"

/*********************************************************************
*
*       GUI__memset
*
* Purpose:
*  Equivalent to the memset function, for 16 bit Data.
*/
void GUI__memset16(uint16_t * p, uint16_t Fill, int NumWords) {
  /* Code for 16 bit CPUs */
  if (sizeof(int) == 2) { /* May some compilers generate a warning at this line: Condition is alwaws true/false */
    if (NumWords >= 8) {  /* May some compilers generate a warning at this line: Unreachable code */
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      *p++ = Fill;
      NumWords -= 8;
    }
    while (NumWords) {
      *p++ = Fill;
      NumWords--;
    }
  } else {
    int NumInts;          /* May some compilers generate a warning at this line: Unreachable code */
    /* Write 16 bit until we are done or have reached an int boundary */
    if (2 & (uint32_t)p) {
      *p++ = Fill;
      NumWords--;
    }
    /* Write Ints */
    NumInts = (unsigned)NumWords / (sizeof(int) / 2);
    if (NumInts) {
      int FillInt;
      int *pInt;
      FillInt = Fill * 0x10001;

      pInt = (int*)p;
      /* Fill large amount of data at a time */
      if (NumInts >= 4) {
        do {
          *pInt       = FillInt;
          *(pInt + 1) = FillInt;
          *(pInt + 2) = FillInt;
          *(pInt + 3) = FillInt;
          pInt += 4;
        } while ((NumInts -= 4) >= 4);
      }
      /* Fill one int at a time */
      while (NumInts) {
        *pInt++ = FillInt;
        NumInts--;
      }
      p = (uint16_t*)pInt;
    }
    /* Fill the remainder */
    if (NumWords & 1) {
      *p = Fill;
    }
  }
}


