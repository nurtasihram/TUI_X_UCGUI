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

struct WM_NOTIFY_CHILD_HAS_FOCUS_INFO {
	WM_Obj *pOld, *pNew;
};

struct WM_CRITICAL_HANDLE {
	WM_CRITICAL_HANDLE *pNext;
	WM_Obj *pWin; 
};

extern GUI_PID_STATE WM_PID__StateLast;

void    WM__ActivateClipRect        (void);
bool    WM__ClipAtParentBorders     (GUI_RECT& r, WM_Obj * pWin);
void    WM__Client2Screen           (const WM_Obj *pWin, GUI_RECT *pRect);
void    WM__DetachWindow            (WM_Obj * pChild);
void    WM__ForEachDesc(WM_Obj * pWin, WM_tfForEach * pcb, void * pData);
WM_Obj * WM__GetFirstSibling         (WM_Obj * pWin);
WM_Obj * WM__GetFocussedChild        (WM_Obj * pWin);
int     WM__GetHasFocus             (WM_Obj * pWin);
WM_Obj * WM__GetLastSibling          (WM_Obj * pWin);
void    WM__InsertWindowIntoList    (WM_Obj * pWin, WM_Obj * pParent);
void    WM__InvalidateAreaBelow     (const GUI_RECT *pRect, WM_Obj * pStopWin);
void    WM__InvalidateTransAreaAbove(const GUI_RECT *pRect, WM_Obj * pStopWin);
bool    WM__IsAncestor              (WM_Obj * pChild, WM_Obj * pParent);
bool    WM__IsAncestorOrSelf        (WM_Obj * pChild, WM_Obj * pParent);
bool    WM__IsChild                 (WM_Obj * pWin, WM_Obj * pParent);
bool    WM__IsInModalArea           (WM_Obj * pWin);
bool    WM__IsInWindow              (WM_Obj * pWin, int x, int y);
void    WM__LeaveIVRSearch          (void);
void    WM__RemoveWindowFromList    (WM_Obj * pWin);
void    WM__RemoveFromLinList       (WM_Obj * pWin);
void    WM__Screen2Client           (const WM_Obj *pWin, GUI_RECT *pRect);
void    WM__UpdateChildPositions    (WM_Obj *pObj, int dx0, int dy0, int dx1, int dy1);

WM_PARAM WM__SendMessage             (WM_Obj * pWin, int MsgId, WM_PARAM Data);

void    WM_PID__GetPrevState        (GUI_PID_STATE *pPrevState);

void    WM__PaintWinAndOverlays     (WM_Obj *pWin);

void    WM__AddCriticalHandle       (WM_CRITICAL_HANDLE *pCH);
void    WM__RemoveCriticalHandle    (WM_CRITICAL_HANDLE *pCH);
