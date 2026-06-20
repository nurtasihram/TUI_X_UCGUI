#pragma once

#include "WM_Intern.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef int GUI_HOOK_FUNC(WM_MESSAGE* pMsg);

typedef struct GUI_HOOK {
  struct GUI_HOOK* pNext;
  GUI_HOOK_FUNC*   pHookFunc;
} GUI_HOOK;

/*********************************************************************
*
*       Functions
*
**********************************************************************
*/

void GUI_HOOK_Add   (GUI_HOOK** ppFirstHook, GUI_HOOK* pNewHook, GUI_HOOK_FUNC* pHookFunc);
void GUI_HOOK_Remove(GUI_HOOK** ppFirstHook, GUI_HOOK* pHook);

#if defined(__cplusplus)
  }
#endif
