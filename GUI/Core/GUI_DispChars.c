

#include "GUI_Protected.h"

void GUI_DispChars(U16P c, int NumChars) {
  while (--NumChars >= 0) {
    GUI_DispChar(c);
  }
}

/*************************** End of file ****************************/
