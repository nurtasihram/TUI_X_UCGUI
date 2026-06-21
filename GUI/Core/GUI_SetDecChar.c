

#include "GUI_Protected.h"
#include "GUIDebug.h"

char GUI_SetDecChar(char c) {
  char r = GUI_DecChar;
  GUI_DecChar = c;
  return r;
}

char GUI_GetDecChar(void) {
  return GUI_DecChar;
}

/*************************** End of file ****************************/
