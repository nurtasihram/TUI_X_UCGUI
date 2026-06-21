

#include "GUI_Protected.h"

int GUI_WaitKey(void) {
  int r;
  do {
    r =  GUI_GetKey();
    if (r) {
      break;
    }
    if (!GUI_Exec()) {
      GUI_X_WAIT_EVENT();      /* Wait for event (keyboard, mouse or whatever) */
    }
  } while (1);
  return r;
}

/*************************** End of file ****************************/
