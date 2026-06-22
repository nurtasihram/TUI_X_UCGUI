

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include "GUI_Protected.h"

int GUI__GetNumChars(const char GUI_UNI_PTR *s) {
  int NumChars = 0;
  if (s) {
    while (GUI_UC__GetCharCodeInc(&s)) {
      NumChars++;
    }
  }
  return NumChars;
}


