

#include "GUI_Protected.h"

void GUI_DispChars(uint16_t c, int NumChars) {
  while (--NumChars >= 0) {
    GUI_DispChar(c);
  }
}


