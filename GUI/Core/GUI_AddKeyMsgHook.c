

#include "GUI_Protected.h"
#include "GUIDebug.h"

GUI_KEY_MSG_HOOK* GUI_SetKeyMsgHook(GUI_KEY_MSG_HOOK* pHook) {
  GUI_KEY_MSG_HOOK* r;
  r = GUI_pfKeyMsgHook;
  GUI_pfKeyMsgHook = pHook;
  return r;
}

/*************************** End of file ****************************/
