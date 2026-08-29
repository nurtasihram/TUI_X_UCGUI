#pragma once

#include "WM.h"
#include "GUI_Protected.h"       /* For GUI */

#if GUI_DEBUG_LEVEL  >= GUI_DEBUG_LEVEL_LOG_WARNINGS
#define WM_ASSERT_NOT_IN_PAINT() { if (WObj::_PaintCallbackCnt) \
									   GUI_DEBUG_ERROROUT("Function may not be called from within a paint event"); \
								   }
#else
#define WM_ASSERT_NOT_IN_PAINT()
#endif

extern PID_STATE WM_PID__StateLast;
PID_STATE WM_PID__GetPrevState        (void);

void    WM__ActivateClipRect        (void);
bool    WM__IsAncestor              (WObj * pChild, WObj * pParent);
bool    WM__IsAncestorOrSelf        (WObj * pChild, WObj * pParent);

WM_PARAM WM__SendMessage             (WObj * pWin, int MsgId, WM_PARAM Data);
