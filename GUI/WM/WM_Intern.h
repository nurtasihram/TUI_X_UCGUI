#pragma once

#include "WM.h"
#include "GUI_Protected.h"       /* For GUI */

#if GUI_DEBUG_LEVEL  >= GUI_DEBUG_LEVEL_LOG_WARNINGS
#define WM_ASSERT_NOT_IN_PAINT() { if (WM__PaintCallbackCnt) \
									   GUI_DEBUG_ERROROUT("Function may not be called from within a paint event"); \
								   }
#else
#define WM_ASSERT_NOT_IN_PAINT()
#endif

extern PID_STATE WM_PID__StateLast;

void    WM__ActivateClipRect        (void);
bool    WM__ClipAtParentBorders     (RECT& r, WObj * pWin);
void    WM__ForEachDesc(WObj * pWin, WM_tfForEach * pcb, void * pData);
WObj * WM__GetFocussedChild        (WObj * pWin);
int     WM__GetHasFocus             (WObj * pWin);
void    WM__InvalidateTransAreaAbove(const RECT *pRect, WObj * pStopWin);
bool    WM__IsAncestor              (WObj * pChild, WObj * pParent);
bool    WM__IsAncestorOrSelf        (WObj * pChild, WObj * pParent);
bool    WM__IsChild                 (WObj * pWin, WObj * pParent);
bool    WM__IsInModalArea           (WObj * pWin);
bool    WM__IsInWindow              (WObj * pWin, int x, int y);
void    WM__LeaveIVRSearch          (void);
void    WM__Screen2Client           (const WObj *pWin, RECT *pRect);

WM_PARAM WM__SendMessage             (WObj * pWin, int MsgId, WM_PARAM Data);

PID_STATE WM_PID__GetPrevState        (void);

void    WM__PaintWinAndOverlays     (WObj *pWin);
