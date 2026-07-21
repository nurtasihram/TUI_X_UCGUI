#pragma once

#include "WM.h"
#include "GUI_Protected.h"       /* For GUI_Context */


/*********************************************************************
*
*       defines
*
**********************************************************************

  The following could be placed in a file of its own as it is not
  used outside of the window manager

*/
/* Basic Windows status flags.
   For module-internally use only !
*/
#define WM_SF_HASTRANS          WM_CF_HASTRANS
#define WM_SF_MEMDEV            WM_CF_MEMDEV
#define WM_SF_MEMDEV_ON_REDRAW  WM_CF_MEMDEV_ON_REDRAW
#define WM_SF_DISABLED          WM_CF_DISABLED  /* Disabled: Does not receive PID (mouse & touch) input */
#define WM_SF_ISVIS             WM_CF_SHOW      /* Is visible flag */

#define WM_SF_STAYONTOP         WM_CF_STAYONTOP
#define WM_SF_LATE_CLIP         WM_CF_LATE_CLIP
#define WM_SF_ANCHOR_RIGHT      WM_CF_ANCHOR_RIGHT
#define WM_SF_ANCHOR_BOTTOM     WM_CF_ANCHOR_BOTTOM
#define WM_SF_ANCHOR_LEFT       WM_CF_ANCHOR_LEFT
#define WM_SF_ANCHOR_TOP        WM_CF_ANCHOR_TOP

#define WM_SF_INVALID           WM_CF_ACTIVATE  /* We reuse this flag, as it is create only and Invalid is status only */

#define WM_SF_CONST_OUTLINE     WM_CF_CONST_OUTLINE       /* Constant outline.*/

#if GUI_DEBUG_LEVEL  >= GUI_DEBUG_LEVEL_LOG_WARNINGS
#define WM_ASSERT_NOT_IN_PAINT() { if (WM__PaintCallbackCnt) \
                                       GUI_DEBUG_ERROROUT("Function may not be called from within a paint event"); \
                                   }
#else
#define WM_ASSERT_NOT_IN_PAINT()
#endif

struct WM_NOTIFY_CHILD_HAS_FOCUS_INFO {
  WM_Obj *pOld;
  WM_Obj *pNew;
};

struct WM_CRITICAL_HANDLE {
  WM_CRITICAL_HANDLE *pNext;
  volatile WM_Obj * pWin;
};

extern GUI_PID_STATE WM_PID__StateLast;

void    WM__ActivateClipRect        (void);
bool    WM__ClipAtParentBorders     (GUI_RECT& r, WM_Obj * pWin);
void    WM__Client2Screen           (const WM_Obj* pWin, GUI_RECT *pRect);
void    WM__DetachWindow            (WM_Obj * pChild);
void    WM__ForEachDesc(WM_Obj * pWin, WM_tfForEach * pcb, void * pData);
WM_Obj * WM__GetFirstSibling         (WM_Obj * pWin);
WM_Obj * WM__GetFocussedChild        (WM_Obj * pWin);
int     WM__GetHasFocus             (WM_Obj * pWin);
WM_Obj * WM__GetLastSibling          (WM_Obj * pWin);
int     WM__GetWindowSizeX          (const WM_Obj* pWin);
int     WM__GetWindowSizeY          (const WM_Obj* pWin);
void    WM__InsertWindowIntoList    (WM_Obj * pWin, WM_Obj * pParent);
void    WM__InvalidateAreaBelow     (const GUI_RECT* pRect, WM_Obj * pStopWin);
void    WM__InvalidateTransAreaAbove(const GUI_RECT* pRect, WM_Obj * pStopWin);
bool    WM__IsAncestor              (WM_Obj * pChild, WM_Obj * pParent);
bool    WM__IsAncestorOrSelf        (WM_Obj * pChild, WM_Obj * pParent);
bool    WM__IsChild                 (WM_Obj * pWin, WM_Obj * pParent);
bool    WM__IsInModalArea           (WM_Obj * pWin);
bool    WM__IsInWindow              (WM_Obj * pWin, int x, int y);
void    WM__LeaveIVRSearch          (void);
void    WM__NotifyVisChanged        (WM_Obj * pWin, GUI_RECT * pRect);
void    WM__RemoveWindowFromList    (WM_Obj * pWin);
void    WM__RemoveFromLinList       (WM_Obj * pWin);
void    WM__Screen2Client           (const WM_Obj* pWin, GUI_RECT *pRect);
void    WM__UpdateChildPositions    (WM_Obj* pObj, int dx0, int dy0, int dx1, int dy1);

void     WM__SendMsgNoData           (WM_Obj * pWin, int MsgId);
WM_PARAM WM__SendMessage             (WM_Obj * pWin, int MsgId, WM_PARAM Data);

void    WM_PID__GetPrevState        (GUI_PID_STATE* pPrevState);

uint16_t WM_GetFlags(WM_Obj * pWin);
void    WM__PaintWinAndOverlays     (WM_Obj *pWin);

void    WM__AddCriticalHandle       (WM_CRITICAL_HANDLE* pCH);
void    WM__RemoveCriticalHandle    (WM_CRITICAL_HANDLE* pCH);

