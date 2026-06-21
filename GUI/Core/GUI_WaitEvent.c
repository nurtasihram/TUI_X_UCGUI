

#include "GUI_Protected.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

void GUI_WaitEvent(void) {
  GUI_X_WAIT_EVENT();
}

/*************************** End of file ****************************/
