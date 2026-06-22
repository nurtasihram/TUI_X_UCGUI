

#include "GUI_Protected.h"
#include "GUIDebug.h"

void GUI_SaveContext(GUI_CONTEXT* pContext) {
  *pContext = GUI_Context;
}

void GUI_RestoreContext(const GUI_CONTEXT* pContext) {
  GUI_Context = *pContext;
}


