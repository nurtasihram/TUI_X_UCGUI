

#include "GUI_Protected.h"
#include "GUIDebug.h"

void GUI_DispHex(U32 v, U8 Len) {
  char ac[9];
	char* s = ac;
  GUI_AddHex(v, Len, &s);
  GUI_DispString(ac);
}

void GUI_DispHexAt(U32 v, I16P x, I16P y, U8 Len) {
  char ac[9];
	char* s = ac;
  GUI_AddHex(v, Len, &s);
  GUI_DispStringAt(ac, x, y);
}

/*************************** End of file ****************************/

