#pragma once

#include "GUI_ConfDefaults.h"

import TUX;
import TUX.Window;

/* Support for transparency. Switching it off makes Wm smaller and faster */
#ifndef WM_SUPPORT_TRANSPARENCY
#define WM_SUPPORT_TRANSPARENCY 1
#endif

/* Make sure we actually have configured windows. If we have not,
there is no point for a windows manager and it will therefor not
generate any code !
*/

#ifndef WM_SUPPORT_OBSTRUCT
#define WM_SUPPORT_OBSTRUCT 1
#endif

/* Send a message if visibility of a window has changed */
#ifndef WM_SUPPORT_NOTIFY_VIS_CHANGED
#define WM_SUPPORT_NOTIFY_VIS_CHANGED 0
#endif

using WM_HMEM = GUI_HMEM;

#define WM_UNATTACHED  ((WM_Obj *)-1)                        /* Do not attach to a window */

struct WM_NOTIFY_INFO {
	int Notification;
	WM_Obj *pWinSrc;
};

typedef void WM_tfForEach(WM_Obj *pWin, void *pData);

void WM_Activate(void);
void WM_Deactivate(void);

void WM_Init(void);
int  WM_Exec(void);  /* Execute all jobs ... Return 0 if nothing was done. */
int  WM_Exec1(void); /* Execute one job  ... Return 0 if nothing was done. */

void WM_SetCapture(WM_Obj *pObj, int AutoRelease);
void WM_SetCaptureMove(WM_Obj *pWin, const GUI_PID_STATE *pState, int MinVisibility); /* Not yet documented */
void WM_ReleaseCapture(void);

uint16_t WM_SetCreateFlags(uint16_t Flags);

void    WM_AttachWindow(WM_Obj *pWin, WM_Obj *pParent);
void    WM_AttachWindowAt(WM_Obj *pWin, WM_Obj *pParent, int x, int y);
int     WM_CheckScrollPos(WM_SCROLL_STATE *pScrollState, int Pos, int LowerDist, int UpperDist); /* not to be documented (may change in future version) */
void    WM_ClrHasTrans(WM_Obj *pWin);
WM_Obj *WM_CreateWindow(int x0, int y0, int xSize, int ySize, uint16_t Style, WM_CALLBACK *cb, int NumExtraBytes);
WM_Obj *WM_CreateWindowAsChild(int x0, int y0, int xSize, int ySize, WM_Obj *pWinParent, uint16_t Style, WM_CALLBACK *cb, int NumExtraBytes);
void    WM_DeleteWindow(WM_Obj *pWin);
void    WM_DetachWindow(WM_Obj *pWin);
int     WM_GetHasTrans(WM_Obj *pWin);
WM_Obj *WM_GetFocussedWindow(void);
void    WM_HideWindow(WM_Obj *pWin);
void    WM_InvalidateArea(const GUI_RECT *pRect);
void    WM_InvalidateRect(WM_Obj *pWin, const GUI_RECT *pRect);
void    WM_Invalidate(WM_Obj *pWin);
void    WM_InvalidateDescs(WM_Obj *pWin);    /* not to be documented (may change in future version) */
bool    WM_IsEnabled(WM_Obj *pObj);
bool    WM_IsFocussable(WM_Obj *pWin);
bool    WM_IsVisible(WM_Obj *pWin);
bool    WM_IsWindow(WM_Obj *pWin);    /* Check validity */
void    WM_SetHasTrans(WM_Obj *pWin);
void    WM_SetId(WM_Obj *pObj, int Id);
void    WM_SetTransState(WM_Obj *pWin, unsigned State);
void    WM_ShowWindow(WM_Obj *pWin);
void    WM_ValidateRect(WM_Obj *pWin, const GUI_RECT *pRect);
void    WM_ValidateWindow(WM_Obj *pWin);
int     WM_GetInvalidRect(WM_Obj *pWin, GUI_RECT *pRect);
void    WM_SetStayOnTop(WM_Obj *pWin, int OnOff);
int     WM_GetStayOnTop(WM_Obj *pWin);
void    WM_SetAnchor(WM_Obj *pWin, uint16_t AnchorFlags);

/* Move/resize windows */
void WM_MoveWindow(WM_Obj *pWin, int dx, int dy);
void WM_ResizeWindow(WM_Obj *pWin, int dx, int dy);
void WM_MoveTo(WM_Obj *pWin, int x, int y);
void WM_MoveChildTo(WM_Obj *pWin, int x, int y);
void WM_SetSize(WM_Obj *pWin, int XSize, int YSize);
int  WM_SetXSize(WM_Obj *pWin, int xSize);
int  WM_SetYSize(WM_Obj *pWin, int ySize);
int  WM_CreateTimer(WM_Obj *pWin, int UserID, int Period, int Mode); /* not to be documented (may change in future version) */
void WM_DeleteTimer(WM_Obj *pWin, int UserId); /* not to be documented (may change in future version) */
bool WM_SetScrollbarH(WM_Obj *pWin, int OnOff); /* not to be documented (may change in future version) */
bool WM_SetScrollbarV(WM_Obj *pWin, int OnOff); /* not to be documented (may change in future version) */

/* Diagnostics */
int WM_GetNumWindows(void);
int WM_GetNumInvalidWindows(void);

/* Scroll state related functions */
int  WM_SetScrollValue(WM_SCROLL_STATE *pScrollState, int v); /* not to be documented (may change in future version) */
void WM_CheckScrollBounds(WM_SCROLL_STATE *pScrollState); /* not to be documented (may change in future version) */

/* Set (new) callback function */
WM_CALLBACK *WM_SetCallback(WM_Obj *Win, WM_CALLBACK *cb);

/* Get size/origin of a window */
GUI_RECT WM_GetClientRect();
GUI_RECT WM_GetClientRect(WM_Obj *pWin);
GUI_RECT WM_GetInsideRect();
GUI_RECT WM_GetInsideRect(WM_Obj *pWin);

void WM_GetInsideRectExScrollbar(WM_Obj *pWin, GUI_RECT *pRect); /* not to be documented (may change in future version) */
WM_Obj *WM_GetFirstChild(WM_Obj *pWin);
WM_Obj *WM_GetNextSibling(WM_Obj *pWin);
WM_Obj *WM_GetParent(WM_Obj *pWin);
WM_Obj *WM_GetPrevSibling(WM_Obj *pWin);
int     WM_GetId(WM_Obj *pWin);
WM_Obj *WM_GetScrollbarV(WM_Obj *pWin);
WM_Obj *WM_GetScrollbarH(WM_Obj *pWin);
WM_Obj *WM_GetScrollPartner(WM_Obj *pWin);
WM_Obj *WM_GetClientWindow(WM_Obj *pObj);
RGBC WM_GetBkColor(WM_Obj *pObj);

/* Change Z-Order of windows */
void WM_BringToBottom(WM_Obj *pWin);
void WM_BringToTop(WM_Obj *pWin);

RGBC WM_SetDesktopColor(RGBC Color);

/* Select window used for drawing operations */
WM_Obj *WM_SelectWindow(WM_Obj *pWin);
WM_Obj *WM_GetActiveWindow(void);
void    WM_Paint(WM_Obj *pObj);

/* Get foreground/background windows */
WM_Obj *WM_GetDesktopWindow(void);

/* Reduce clipping area of a window */
const GUI_RECT *WM_SetUserClipRect(const GUI_RECT *pRect);

/* Use of memory devices */
void WM_EnableMemdev(WM_Obj *pWin);
void WM_DisableMemdev(WM_Obj *pWin);

int WM_OnKey(int Key, int Pressed);

/******************************************************************
*
*           Message related funcions
*
*******************************************************************
	Please note that some of these functions do not yet show up in the
	documentation, as they should not be required by application program.
*/

void      WM_NotifyParent(WM_Obj *pWin, int Notification);
WM_PARAM  WM_SendMessage(WM_Obj *pWin, int MsgId, WM_PARAM Data);
void      WM_SendMessageNoPara(WM_Obj *pWin, int MsgId); /* not to be documented (may change in future versionumented */

WM_PARAM  WM_DefaultProc(WM_Obj *pWin, int MsgId, WM_PARAM Data);

void      WM_SetScrollState(WM_Obj *pWin, const WM_SCROLL_STATE *pState);
void      WM_SetEnableState(WM_Obj *pItem, int State);
bool      WM_HasCaptured(WM_Obj *pWin);
bool      WM_HasFocus(WM_Obj *pWin);
int       WM_SetFocus(WM_Obj *pWin);
WM_Obj *WM_SetFocusOnNextChild(WM_Obj *pParent);     /* Set the focus to the next child */
WM_Obj *WM_SetFocusOnPrevChild(WM_Obj *pParent);     /* Set the focus to the previous child */
WM_Obj *WM_GetDialogItem(WM_Obj *pWin, int Id);
void      WM_GetScrollState(WM_Obj *pObj, WM_SCROLL_STATE *pScrollState);

int       WM_HandlePID(void);
WM_Obj *WM_Screen2hWin(int x, int y);
WM_Obj *WM_Screen2hWinEx(WM_Obj *pStop, int x, int y);
void      WM_ForEachDesc(WM_Obj *pWin, WM_tfForEach *pcb, void *pData);

#pragma region IVR
bool WM__InitIVRSearch(GUI_RECT rcMax);
bool WM__GetNextIVR(void);
inline void WM_Iterate(GUI_RECT &r, auto fn) {
	if (WM__InitIVRSearch(r))
		do { fn(); }
		while (WM__GetNextIVR());

}
#pragma endregion
