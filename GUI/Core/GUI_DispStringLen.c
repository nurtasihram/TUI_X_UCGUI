

#include "GUI_Protected.h"

void GUI_DispStringLen(const char GUI_UNI_PTR *s, int MaxNumChars) {
  uint16_t Char;

  while (MaxNumChars && ((Char = GUI_UC__GetCharCodeInc(&s)) != 0)) {
    GUI_DispChar(Char);
    MaxNumChars--;
  }
  while (MaxNumChars--) {
    GUI_DispChar(' ');
  }

}


