#include "GUI_Protected.h"

int GUI_GetTime(void) {
  return GUI_X_GetTime();
}

void GUI_Delay(int Period) {
  int EndTime = GUI_GetTime()+Period;
  int tRem; /* remaining Time */
  while (tRem = EndTime- GUI_GetTime(), tRem>0) {
    GUI_Exec();
    GUI_X_Delay((tRem >5) ? 5 : tRem);
  }
}
