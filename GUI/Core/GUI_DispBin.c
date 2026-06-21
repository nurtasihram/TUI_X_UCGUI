

#include "GUI_Protected.h"
#include "GUIDebug.h"
#include "string.h"

void GUI_DispBin(U32 v, U8 Len) {
	char ac[33];
	char* s = ac;
  GUI_AddBin(v, Len, &s);
  GUI_DispString(ac);
}

void GUI_DispBinAt(U32 v, I16P x, I16P y, U8 Len) {
	char ac[33];
	char* s = ac;
  GUI_AddBin(v, Len, &s);
  GUI_DispStringAt(ac, x, y);
}

/*************************** End of file ****************************/
