

#include "GUI_Protected.h"
#include "GUIDebug.h"
#include "string.h"

void GUI_DispDecShift(I32 v, U8 Len, U8 Shift) {
	char ac[12];
	char* s = ac;
  GUI_AddDecShift(v, Len, Shift, &s);
  GUI_DispString(ac);
}

void GUI_DispSDecShift(I32 v, U8 Len, U8 Shift) {
	char ac[12];
	char* s = ac;
  v = GUI_AddSign(v, &s);
  GUI_AddDecShift(v, --Len, Shift, &s);
  GUI_DispString(ac);
}

void GUI_DispDecMin(I32 v) {
	char ac[12];
	char* s = ac;
  GUI_AddDecMin(v, &s);
  GUI_DispString(ac);
}

void GUI_DispDecSpace(I32 v, U8 MaxDigits) {
	char ac[12];
	char* s = ac;
  U8 Len;
  if (v < 0) {
    v = GUI_AddSign(v, &s);
    MaxDigits--;
  }
  Len = GUI_Long2Len(v);
  while (Len++ < MaxDigits) {
    *s++ = ' ';
  }
  GUI_AddDecMin(v, &s);
  GUI_DispString(ac);
}

void GUI_DispSDec(I32 v, U8 Len) {
  GUI_DispSDecShift(v,Len,0);
}

void GUI_DispDec(I32 v, U8 Len) {
  GUI_DispDecShift(v, Len,0);
}

void GUI_DispDecAt(I32 v, I16P x, I16P y, U8 Len) {
	char ac[12];
	char* s = ac;
  GUI_AddDecShift(v, Len, 0, &s);
  GUI_DispStringAt(ac, x, y);
}

/*************************** End of file ****************************/
