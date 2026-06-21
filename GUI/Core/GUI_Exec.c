

#include "GUI_Protected.h"
#include "WM.h"

int GUI_Exec1(void) {
  int r = 0;
  /* Execute background jobs */
  if (GUI_pfTimerExec) {
    if ((*GUI_pfTimerExec)()) {
      r = 1;                  /* We have done something */
    }
  }
  if (WM_Exec())
    r = 1;
  return r;
}

int GUI_Exec(void) {
  int r = 0;
  while (GUI_Exec1()) {
    r = 1;                  /* We have done something */
  }
  return r;
}

/*************************** End of file ****************************/
