

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LCD_Private.h"      /* private modul definitions & config */
#include "GUI_Protected.h"

#if GUI_WINSUPPORT
#include "WM.h"
#endif

static GUI_PID_STATE _State;

/*********************************************************************
*
*       _PID_Load
*
* Purpose:
*   If the window manager is available, write the function pointer for
*   the PID handler.
*/
static void _PID_Load(void) {
#if (GUI_WINSUPPORT)
    WM_pfHandlePID = WM_HandlePID;
#endif
}

int GUI_PID_GetState(GUI_PID_STATE *pState) {
  *pState = _State;
  return (pState->Pressed != 0) ? 1 : 0;
}

void GUI_PID_StoreState(const GUI_PID_STATE *pState) {
  _PID_Load();
  if (memcmp(&_State, pState, sizeof(_State))) {
    _State = *pState;
  }
}

/*************************** End of file ****************************/
