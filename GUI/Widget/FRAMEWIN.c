#include <stdlib.h>
#include <string.h>

#include "GUI_Protected.h"
#include "WM_Intern.h"

#include "WIDGET.h"
#include "MENU.h"
#include "BUTTON.h"

#include "FRAMEWIN.h"
#include "FRAMEWIN_Private.h"

/* Support for 3D effects */
#ifndef FRAMEWIN_CLIENTCOLOR_DEFAULT
#define FRAMEWIN_CLIENTCOLOR_DEFAULT 0xc0c0c0
#endif
/* Default for top frame size */
#ifndef FRAMEWIN_TITLEHEIGHT_DEFAULT
#define FRAMEWIN_TITLEHEIGHT_DEFAULT 0
#endif
/* Default for left/right/top/bottom frame size */
#ifndef FRAMEWIN_BORDER_DEFAULT
#define FRAMEWIN_BORDER_DEFAULT 3
#endif
/* Default for inner frame size */
#ifndef FRAMEWIN_IBORDER_DEFAULT
#define FRAMEWIN_IBORDER_DEFAULT 1
#endif
/* Default font */
#ifndef FRAMEWIN_DEFAULT_FONT
#define FRAMEWIN_DEFAULT_FONT &GUI_Font8_1
#endif
/* Default barcolor when framewin is active */
#ifndef FRAMEWIN_BARCOLOR_ACTIVE_DEFAULT
#define FRAMEWIN_BARCOLOR_ACTIVE_DEFAULT 0xFF0000
#endif
/* Default barcolor when framewin is inactive */
#ifndef FRAMEWIN_BARCOLOR_INACTIVE_DEFAULT
#define FRAMEWIN_BARCOLOR_INACTIVE_DEFAULT 0x404040
#endif
/* Default framecolor */
#ifndef FRAMEWIN_FRAMECOLOR_DEFAULT
#define FRAMEWIN_FRAMECOLOR_DEFAULT 0xAAAAAA
#endif
/* Default textcolor when framewin is active */
#ifndef FRAMEWIN_TEXTCOLOR0_DEFAULT
#define FRAMEWIN_TEXTCOLOR0_DEFAULT GUI_WHITE
#endif
/* Default textcolor when framewin is inactive */
#ifndef FRAMEWIN_TEXTCOLOR1_DEFAULT
#define FRAMEWIN_TEXTCOLOR1_DEFAULT GUI_WHITE
#endif
FRAMEWIN_PROPS FRAMEWIN__DefaultProps = {
  FRAMEWIN_DEFAULT_FONT,
  FRAMEWIN_BARCOLOR_INACTIVE_DEFAULT,
  FRAMEWIN_BARCOLOR_ACTIVE_DEFAULT,
  FRAMEWIN_TEXTCOLOR0_DEFAULT,
  FRAMEWIN_TEXTCOLOR1_DEFAULT,
  FRAMEWIN_CLIENTCOLOR_DEFAULT,
  FRAMEWIN_TITLEHEIGHT_DEFAULT,
  FRAMEWIN_BORDER_DEFAULT,
  FRAMEWIN_IBORDER_DEFAULT
};
static int16_t FRAMEWIN__MinVisibility = 5;
static void _SetActive(FRAMEWIN_Handle hObj, int State) {
	FRAMEWIN_Obj *pObj;
	pObj = FRAMEWIN_H2P(hObj);
	if (State && !(pObj->Flags & FRAMEWIN_SF_ACTIVE)) {
		pObj->Flags |= FRAMEWIN_CF_ACTIVE;
		FRAMEWIN_Invalidate(hObj);
	}
	else if (!State && (pObj->Flags & FRAMEWIN_SF_ACTIVE)) {
		pObj->Flags &= ~FRAMEWIN_CF_ACTIVE;
		FRAMEWIN_Invalidate(hObj);
	}
}
static void _OnTouch(FRAMEWIN_Handle hWin, FRAMEWIN_Obj *pObj, WM_MESSAGE *pMsg) {
	const GUI_PID_STATE *pState;
	pState = (const GUI_PID_STATE *)pMsg->Data.p;
	if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			if (!(pObj->Flags & FRAMEWIN_SF_ACTIVE)) {
				WM_SetFocus(hWin);
			}
			WM_BringToTop(hWin);
			if (pObj->Flags & FRAMEWIN_SF_MOVEABLE) {
				WM_SetCaptureMove(hWin, pState, FRAMEWIN__MinVisibility);
			}
		}
	}
}
/*********************************************************************
*
*       _Paint  (Frame)
*
*/
static void _Paint(FRAMEWIN_Obj *pObj) {
	WM_HWIN hWin = WM_GetActiveWindow();
	const char *pText = NULL;
	int xsize = WM_GetWindowSizeX(hWin);
	int ysize = WM_GetWindowSizeY(hWin);
	int BorderSize = pObj->Props.BorderSize;
	int y0, Index;
	POSITIONS Pos;
	GUI_RECT r, rText;
	/* Perform computations */
	FRAMEWIN__CalcPositions(pObj, &Pos);
	Index = (pObj->Flags & FRAMEWIN_SF_ACTIVE) ? 1 : 0;
	if (pObj->hText) {
		pText = (const char *)GUI_ALLOC_h2p(pObj->hText);
	}
	r.x0 = Pos.rClient.x0;
	r.x1 = Pos.rClient.x1;
	r.y0 = Pos.rTitleText.y0;
	r.y1 = Pos.rTitleText.y1;
	Pos.rTitleText.y0++;
	Pos.rTitleText.x0++;
	Pos.rTitleText.x1--;
	GUI_SetFont(pObj->Props.pFont);
	GUI__CalcTextRect(pText, &Pos.rTitleText, &rText, pObj->TextAlign);
	y0 = Pos.TitleHeight + BorderSize;
	/* Perform drawing operations */
	WM_ITERATE_START(NULL) {
		/* Draw Title */
		LCD_SetBkColor(pObj->Props.aBarColor[Index]);
		LCD_SetColor(pObj->Props.aTextColor[Index]);
		WIDGET__FillStringInRect(pText, &r, &Pos.rTitleText, &rText);
		/* Draw Frame */
		LCD_SetColor(FRAMEWIN_FRAMECOLOR_DEFAULT);
		GUI_FillRect(0, 0, xsize - 1, BorderSize - 1);
		GUI_FillRect(0, 0, Pos.rClient.x0 - 1, ysize - 1);
		GUI_FillRect(Pos.rClient.x1 + 1, 0, xsize - 1, ysize - 1);
		GUI_FillRect(0, Pos.rClient.y1 + 1, xsize - 1, ysize - 1);
		GUI_FillRect(0, y0, xsize - 1, y0 + pObj->Props.IBorderSize - 1);
		/* Draw the 3D effect (if configured) */
		if (pObj->Props.BorderSize >= 2) {
			WIDGET_EFFECT_3D_DrawUp();  /* pObj->Widget.pEffect->pfDrawUp(); */
		}
	} WM_ITERATE_END();
}
/*********************************************************************
*
*       _OnChildHasFocus
*
* Function:
*   A child has received or lost the focus.
*   The basic idea is to make sure the framewindow is active if a
*   descendant has the focus.
*   If the focus travels from one desc. to an other, there is no need
*   to make the framewindow inactive and active again.
*   Avoiding this complicates the code a litlle, but avoids flicker
*   and waste of CPU load.
*
*/
static void _OnChildHasFocus(FRAMEWIN_Handle hWin, FRAMEWIN_Obj *pObj, WM_MESSAGE *pMsg) {
	if (pMsg->Data.p) {
		const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *pInfo = (const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *)pMsg->Data.p;
		int IsDesc = WM__IsAncestorOrSelf(pInfo->hNew, hWin);
		if (IsDesc) {                         /* A child has received the focus, Framewindow needs to be activated */
			_SetActive(hWin, 1);
		}
		else {                  /* A child has lost the focus, we need to deactivate */
			_SetActive(hWin, 0);
			/* Remember the child which had the focus so we can reactive this child */
			if (WM__IsAncestor(pInfo->hOld, hWin)) {
				pObj->hFocussedChild = pInfo->hOld;
			}
		}
	}
}
static void _FRAMEWIN_Callback(WM_MESSAGE *pMsg) {
	FRAMEWIN_Handle hWin = (FRAMEWIN_Handle)(pMsg->hWin);
	FRAMEWIN_Obj *pObj = FRAMEWIN_H2P(hWin);
	GUI_RECT *pRect = (GUI_RECT *)(pMsg->Data.p);
	POSITIONS Pos;
	GUI_HOOK *pHook;
	/* Call hook functions */
	for (pHook = pObj->pFirstHook; pHook; pHook = pHook->pNext) {
		int r;
		r = (*pHook->pHookFunc)(pMsg);
		if (r) {
			return;   /* Message handled */
		}
	}
	switch (pMsg->MsgId) {
		case WM_HANDLE_DIALOG_STATUS:
			if (pMsg->Data.p) {                           /* set pointer to Dialog status */
				pObj->pDialogStatus = (WM_DIALOG_STATUS *)pMsg->Data.p;
			}
			else {                                      /* return pointer to Dialog status */
				pMsg->Data.p = pObj->pDialogStatus;
			}
			return;
		case WM_PAINT:
			_Paint(pObj);
			break;
		case WM_TOUCH:
			_OnTouch(hWin, pObj, pMsg);
			return;                       /* Return here ... Message handled */
		case WM_GET_INSIDE_RECT:
			FRAMEWIN__CalcPositions(pObj, &Pos);
			*pRect = Pos.rClient;
			return;                       /* Return here ... Message handled */
		case WM_GET_CLIENT_WINDOW:      /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			pMsg->Data.v = (int)pObj->hClient;
			return;                       /* Return here ... Message handled */
		case WM_NOTIFY_PARENT:
			if (pMsg->Data.v == WM_NOTIFICATION_RELEASED) {
				WM_MESSAGE Msg;
				Msg.hWinSrc = hWin;
				Msg.Data = pMsg->Data;
				Msg.MsgId = WM_NOTIFY_PARENT_REFLECTION;
				WM_SendMessage(pMsg->hWinSrc, &Msg);
			}
			return;
		case WM_SET_FOCUS:                 /* We have received or lost focus */
			if (pMsg->Data.v == 1) {
				if (WM_IsWindow(pObj->hFocussedChild)) {
					WM_SetFocus(pObj->hFocussedChild);
				}
				else {
					pObj->hFocussedChild = WM_SetFocusOnNextChild(pObj->hClient);
				}
				FRAMEWIN_SetActive(hWin, 1);
				pMsg->Data.v = 0;              /* Focus could be accepted */
			}
			else {
				FRAMEWIN_SetActive(hWin, 0);
			}
			return;
		case WM_TOUCH_CHILD:
			/* If a child of this framewindow has been touched and the frame window was not active,
			   the framewindow will receive the focus.
			 */
			if (!(pObj->Flags & FRAMEWIN_SF_ACTIVE)) {
				const WM_MESSAGE *pMsgOrg;
				const GUI_PID_STATE *pState;
				pMsgOrg = (const WM_MESSAGE *)pMsg->Data.p;      /* The original touch message */
				pState = (const GUI_PID_STATE *)pMsgOrg->Data.p;
				if (pState) {          /* Message may not have a valid pointer (moved out) ! */
					if (pState->Pressed) {
						WM_SetFocus(hWin);
					}
				}
			}
			break;
		case WM_NOTIFY_CHILD_HAS_FOCUS:
			_OnChildHasFocus(hWin, pObj, pMsg);
			break;
		case WM_DELETE:
			GUI_DEBUG_LOG("FRAMEWIN: _FRAMEWIN_Callback(WM_DELETE)\n");
			GUI_ALLOC_FreePtr(&pObj->hText);
			break;
	}
	/* Let widget handle the standard messages */
	if (WIDGET_HandleActive(hWin, pMsg) == 0) {
		return;
	}
	WM_DefaultProc(pMsg);
}
static void FRAMEWIN__cbClient(WM_MESSAGE *pMsg) {
	WM_HWIN hWin = pMsg->hWin;
	WM_HWIN hParent = WM_GetParent(pMsg->hWin);
	FRAMEWIN_Obj *pObj = FRAMEWIN_H2P(hParent);
	WM_CALLBACK *cb = pObj->cb;
	switch (pMsg->MsgId) {
		case WM_PAINT:
			if (pObj->Props.ClientColor != GUI_INVALID_COLOR) {
				LCD_SetBkColor(pObj->Props.ClientColor);
				GUI_Clear();
			}
			/* Give the user callback  a chance to draw.
			 * Note that we can not run into the bottom part, as this passes the parents handle
			 */
			if (cb) {
				WM_MESSAGE Msg;
				Msg = *pMsg;
				Msg.hWin = hWin;
				(*cb)(&Msg);
			}
			return;
		case WM_SET_FOCUS:
			if (pMsg->Data.v) {     /* Focus received */
				if (pObj->hFocussedChild && (pObj->hFocussedChild != hWin)) {
					WM_SetFocus(pObj->hFocussedChild);
				}
				else {
					pObj->hFocussedChild = WM_SetFocusOnNextChild(hWin);
				}
				pMsg->Data.v = 0;     /* Focus change accepted */
			}
			return;
		case WM_GET_ACCEPT_FOCUS:
			WIDGET_HandleActive(hParent, pMsg);
			return;
		case WM_KEY:
			if (((const WM_KEY_INFO *)(pMsg->Data.p))->PressedCnt > 0) {
				int Key = ((const WM_KEY_INFO *)(pMsg->Data.p))->Key;
				switch (Key) {
					case GUI_KEY_TAB:
						pObj->hFocussedChild = WM_SetFocusOnNextChild(hWin);
						return;
				}
			}
			break;	                       /* Send to parent by not doing anything */
		case WM_GET_BKCOLOR:
			pMsg->Data.Color = pObj->Props.ClientColor;
			return;                       /* Message handled */
		case WM_GET_INSIDE_RECT:        /* This should not be passed to parent ... (We do not want parents coordinates)*/
		case WM_GET_ID:                 /* This should not be passed to parent ... (Possible recursion problem)*/
		case WM_GET_CLIENT_WINDOW:      /* return handle to client window. For most windows, there is no seperate client window, so it is the same handle */
			WM_DefaultProc(pMsg);
			return;                       /* We are done ! */
	}
	/* Call user callback. Note that the user callback gets the handle of the Framewindow itself, NOT the Client. */
	if (cb) {
		pMsg->hWin = hParent;
		(*cb)(pMsg);
	}
	else {
		WM_DefaultProc(pMsg);
	}
}
int FRAMEWIN__CalcTitleHeight(FRAMEWIN_Obj *pObj) {
	int r = 0;
	if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
		r = pObj->Props.TitleHeight;
		if (r == 0) {
			r = 2 + GUI_GetYSizeOfFont(pObj->Props.pFont);
		}
	}
	return r;
}
void FRAMEWIN__CalcPositions(FRAMEWIN_Obj *pObj, POSITIONS *pPos) {
	WM_HWIN hChild;
	WM_Obj *pChild;
	int TitleHeight;
	int MenuHeight = 0;
	int IBorderSize = 0;
	int BorderSize;
	int xsize, ysize;
	int x0, x1, y0;
	BorderSize = pObj->Props.BorderSize;
	xsize = WM__GetWindowSizeX(&pObj->Widget.Win);
	ysize = WM__GetWindowSizeY(&pObj->Widget.Win);
	if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
		IBorderSize = pObj->Props.IBorderSize;
	}
	TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
	if (pObj->hMenu) {
		MenuHeight = WM_GetWindowSizeY(pObj->hMenu);
	}
	pPos->TitleHeight = TitleHeight;
	pPos->MenuHeight = MenuHeight;
	/* Set object properties accordingly */
	pPos->rClient.x0 = BorderSize;
	pPos->rClient.x1 = xsize - BorderSize - 1;
	pPos->rClient.y0 = BorderSize + IBorderSize + TitleHeight + MenuHeight;
	pPos->rClient.y1 = ysize - BorderSize - 1;
	/* Calculate title rect */
	pPos->rTitleText.x0 = BorderSize;
	pPos->rTitleText.x1 = xsize - BorderSize - 1;
	pPos->rTitleText.y0 = BorderSize;
	pPos->rTitleText.y1 = BorderSize + TitleHeight - 1;
	/* Iterate over all children */
	for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = pChild->hNext) {
		pChild = WM_H2P(hChild);
		x0 = pChild->Rect.x0 - pObj->Widget.Win.Rect.x0;
		x1 = pChild->Rect.x1 - pObj->Widget.Win.Rect.x0;
		y0 = pChild->Rect.y0 - pObj->Widget.Win.Rect.y0;
		if (y0 == BorderSize) {
			if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
				if (x0 <= pPos->rTitleText.x1) {
					pPos->rTitleText.x1 = x0 - 1;
				}
			}
			else {
				if (x1 >= pPos->rTitleText.x0) {
					pPos->rTitleText.x0 = x1 + 1;
				}
			}
		}
	}
}
void FRAMEWIN__UpdatePositions(FRAMEWIN_Obj *pObj) {
	/* Move client window accordingly */
	if (pObj->hClient || pObj->hMenu) {
		POSITIONS Pos;
		FRAMEWIN__CalcPositions(pObj, &Pos);
		if (pObj->hClient) {
			WM_MoveChildTo(pObj->hClient, Pos.rClient.x0, Pos.rClient.y0);
			WM_SetSize(pObj->hClient,
					   Pos.rClient.x1 - Pos.rClient.x0 + 1,
					   Pos.rClient.y1 - Pos.rClient.y0 + 1);
		}
		if (pObj->hMenu) {
			WM_MoveChildTo(pObj->hMenu, Pos.rClient.x0, Pos.rClient.y0 - Pos.MenuHeight);
		}
	}
}
FRAMEWIN_Handle FRAMEWIN_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
								  int WinFlags, int ExFlags, int Id, const char *pTitle, WM_CALLBACK *cb) {
	FRAMEWIN_Handle hObj;
	/* Create the window */
	WinFlags |= WM_CF_LATE_CLIP;    /* Always use late clipping since widget is optimized for it. */
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, _FRAMEWIN_Callback,
								  sizeof(FRAMEWIN_Obj) - sizeof(WM_Obj));
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		POSITIONS Pos;

		pObj = FRAMEWIN_H2P(hObj);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE | FRAMEWIN_SF_TITLEVIS);
		/* init member variables */
		pObj->Props = FRAMEWIN__DefaultProps;
		pObj->TextAlign = GUI_TA_LEFT;
		pObj->cb = cb;
		pObj->Flags = ExFlags;
		pObj->hFocussedChild = 0;
		pObj->hMenu = 0;
		pObj->pFirstHook = NULL;
		FRAMEWIN__CalcPositions(pObj, &Pos);
		pObj->hClient = WM_CreateWindowAsChild(Pos.rClient.x0, Pos.rClient.y0,
											   Pos.rClient.x1 - Pos.rClient.x0 + 1,
											   Pos.rClient.y1 - Pos.rClient.y0 + 1,
											   hObj,
											   WM_CF_ANCHOR_RIGHT | WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_TOP | WM_CF_ANCHOR_BOTTOM | WM_CF_SHOW | WM_CF_LATE_CLIP,
											   FRAMEWIN__cbClient, 0);
		/* Normally we disable memory devices for the frame window:
		 * The frame window does not flicker, and not using memory devices is usually faster.
		 * You can still use memory by explicitly specifying the flag
		 */
		if ((WinFlags & (WM_CF_MEMDEV | (WM_CF_MEMDEV_ON_REDRAW))) == 0) {
			WM_DisableMemdev(hObj);
		}
		FRAMEWIN_SetText(hObj, pTitle);

	}
	return hObj;
}
void FRAMEWIN_SetText(FRAMEWIN_Handle hObj, const char *s) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;

		pObj = FRAMEWIN_H2P(hObj);
		if (GUI__SetText(&pObj->hText, s)) {
			FRAMEWIN_Invalidate(hObj);
		}

	}
}
void FRAMEWIN_SetTextAlign(FRAMEWIN_Handle hObj, int Align) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;

		pObj = FRAMEWIN_H2P(hObj);
		if (pObj->TextAlign != Align) {
			pObj->TextAlign = Align;
			FRAMEWIN_Invalidate(hObj);
		}

	}
}
void FRAMEWIN_SetMoveable(FRAMEWIN_Handle hObj, int State) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;

		pObj = FRAMEWIN_H2P(hObj);
		if (State) {
			pObj->Flags |= FRAMEWIN_CF_MOVEABLE;
		}
		else {
			pObj->Flags &= ~FRAMEWIN_CF_MOVEABLE;
		}

	}
}
void FRAMEWIN_SetActive(FRAMEWIN_Handle hObj, int State) {
	if (hObj) {

		_SetActive(hObj, State);

	}
}

void FRAMEWIN_AddMenu(FRAMEWIN_Handle hObj, WM_HWIN hMenu) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		if (pObj) {
			int TitleHeight, BorderSize, IBorderSize = 0;
			int x0, y0, xSize;
			TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
			BorderSize = pObj->Props.BorderSize;
			if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
				IBorderSize = pObj->Props.IBorderSize;
			}
			x0 = BorderSize;
			y0 = BorderSize + TitleHeight + IBorderSize;
			xSize = WM__GetWindowSizeX(&pObj->Widget.Win);
			xSize -= BorderSize * 2;
			pObj->hMenu = hMenu;
			if (pObj->cb) {
				MENU_SetOwner(hMenu, pObj->hClient);
			}
			MENU_Attach(hMenu, hObj, x0, y0, xSize, 0, 0);
			WM_SetAnchor(hMenu, WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_RIGHT);
			FRAMEWIN__UpdatePositions(pObj);
			FRAMEWIN_Invalidate(hObj);
		}
	}
}


FRAMEWIN_Handle FRAMEWIN_Create(const char *pText, WM_CALLBACK *cb, int Flags,
								int x0, int y0, int xsize, int ysize) {
	return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, WM_HWIN_NULL, Flags, 0, 0, pText, cb);
}
FRAMEWIN_Handle FRAMEWIN_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
									   const char *pText, WM_CALLBACK *cb, int Flags) {
	return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, 0, pText, cb);
}
FRAMEWIN_Handle FRAMEWIN_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent,
										int x0, int y0, WM_CALLBACK *cb) {
	FRAMEWIN_Handle hObj;
	hObj = FRAMEWIN_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							 hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->pName, cb);
	return hObj;
}
void FRAMEWIN_SetDefaultFont(const GUI_FONT GUI_UNI_PTR *pFont) {
	FRAMEWIN__DefaultProps.pFont = pFont;
}
const GUI_FONT GUI_UNI_PTR *FRAMEWIN_GetDefaultFont(void) {
	return FRAMEWIN__DefaultProps.pFont;
}
void FRAMEWIN_SetDefaultBarColor(unsigned Index, RGB_COLOR Color) {
	if (Index < GUI_COUNTOF(FRAMEWIN__DefaultProps.aBarColor)) {
		FRAMEWIN__DefaultProps.aBarColor[Index] = Color;
	}
}
RGB_COLOR FRAMEWIN_GetDefaultBarColor(unsigned Index) {
	RGB_COLOR Color = 0;
	if (Index < GUI_COUNTOF(FRAMEWIN__DefaultProps.aBarColor)) {
		Color = FRAMEWIN__DefaultProps.aBarColor[Index];
	}
	return Color;
}
void FRAMEWIN_SetDefaultClientColor(RGB_COLOR Color) {
	FRAMEWIN__DefaultProps.ClientColor = Color;
}
RGB_COLOR FRAMEWIN_GetDefaultClientColor(void) {
	return FRAMEWIN__DefaultProps.ClientColor;
}
void FRAMEWIN_SetDefaultTitleHeight(int Height) {
	FRAMEWIN__DefaultProps.TitleHeight = Height;
}
int FRAMEWIN_GetDefaultTitleHeight(void) {
	return FRAMEWIN__DefaultProps.TitleHeight;
}
void FRAMEWIN_SetDefaultBorderSize(int DefaultBorderSize) {
	FRAMEWIN__DefaultProps.BorderSize = DefaultBorderSize;
}
int FRAMEWIN_GetDefaultBorderSize(void) {
	return FRAMEWIN__DefaultProps.BorderSize;
}
void FRAMEWIN_SetDefaultTextColor(unsigned Index, RGB_COLOR Color) {
	if (Index < GUI_COUNTOF(FRAMEWIN__DefaultProps.aTextColor)) {
		FRAMEWIN__DefaultProps.aTextColor[Index] = Color;
	}
}
RGB_COLOR FRAMEWIN_GetDefaultTextColor(unsigned Index) {
	RGB_COLOR Color = 0;
	if (Index < GUI_COUNTOF(FRAMEWIN__DefaultProps.aTextColor)) {
		Color = FRAMEWIN__DefaultProps.aTextColor[Index];
	}
	return Color;
}

const GUI_FONT GUI_UNI_PTR *FRAMEWIN_GetFont(FRAMEWIN_Handle hObj) {
	const GUI_FONT GUI_UNI_PTR *r = NULL;
	if (hObj) {
		FRAMEWIN_Obj *pObj = FRAMEWIN_H2P(hObj);
		r = pObj->Props.pFont;
		FRAMEWIN_Invalidate(hObj);
	}
	return r;
}
int FRAMEWIN_GetTitleHeight(FRAMEWIN_Handle hObj) {
	int r = 0;
	POSITIONS Pos;
	/* Move client window accordingly */
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		FRAMEWIN__CalcPositions(pObj, &Pos);
		r = pObj->Props.TitleHeight;
		if (r == 0) {
			r = Pos.TitleHeight;
		}
	}
	return r;
}
int FRAMEWIN_GetBorderSize(FRAMEWIN_Handle hObj) {
	int r = 0;
	/* Move client window accordingly */
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		r = pObj->Props.BorderSize;
	}
	return r;
}

int FRAMEWIN_IsMinimized(FRAMEWIN_Handle hObj) {
	int r = 0;
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		r = (pObj->Flags & FRAMEWIN_SF_MINIMIZED) ? 1 : 0;
	}
	return r;
}
int FRAMEWIN_IsMaximized(FRAMEWIN_Handle hObj) {
	int r = 0;
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		r = (pObj->Flags & FRAMEWIN_SF_MAXIMIZED) ? 1 : 0;
	}
	return r;
}

static void _InvalidateButton(FRAMEWIN_Obj *pObj, int Id) {
	WM_HWIN hChild;
	WM_Obj *pChild;
	for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = pChild->hNext) {
		pChild = WM_H2P(hChild);
		if (WM_GetId(hChild) == Id) {
			WM_InvalidateWindow(hChild);
		}
	}
}
static void _RestoreMinimized(FRAMEWIN_Handle hObj, FRAMEWIN_Obj *pObj) {
	/* When window was minimized, restore it */
	if (pObj->Flags & FRAMEWIN_SF_MINIMIZED) {
		int OldHeight = 1 + pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0;
		int NewHeight = 1 + pObj->rRestore.y1 - pObj->rRestore.y0;
		WM_ResizeWindow(hObj, 0, NewHeight - OldHeight);
		WM_ShowWindow(pObj->hClient);
		WM_ShowWindow(pObj->hMenu);
		FRAMEWIN__UpdatePositions(pObj);
		pObj->Flags &= ~FRAMEWIN_SF_MINIMIZED;
		_InvalidateButton(pObj, GUI_ID_MINIMIZE);
	}
}
static void _RestoreMaximized(FRAMEWIN_Handle hObj, FRAMEWIN_Obj *pObj) {
	/* When window was maximized, restore it */
	if (pObj->Flags & FRAMEWIN_SF_MAXIMIZED) {
		GUI_RECT r = pObj->rRestore;
		WM_MoveTo(hObj, r.x0, r.y0);
		WM_SetSize(hObj, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
		FRAMEWIN__UpdatePositions(pObj);
		pObj->Flags &= ~FRAMEWIN_SF_MAXIMIZED;
		_InvalidateButton(pObj, GUI_ID_MAXIMIZE);
	}
}
static void _MinimizeFramewin(FRAMEWIN_Handle hObj, FRAMEWIN_Obj *pObj) {
	_RestoreMaximized(hObj, pObj);
	/* When window is not minimized, minimize it */
	if ((pObj->Flags & FRAMEWIN_SF_MINIMIZED) == 0) {
		int OldHeight = pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1;
		int NewHeight = FRAMEWIN__CalcTitleHeight(pObj) + pObj->Widget.pEffect->EffectSize * 2 + 2;
		pObj->rRestore = pObj->Widget.Win.Rect;
		WM_HideWindow(pObj->hClient);
		WM_HideWindow(pObj->hMenu);
		WM_ResizeWindow(hObj, 0, NewHeight - OldHeight);
		FRAMEWIN__UpdatePositions(pObj);
		pObj->Flags |= FRAMEWIN_SF_MINIMIZED;
		_InvalidateButton(pObj, GUI_ID_MINIMIZE);
	}
}
static void _MaximizeFramewin(FRAMEWIN_Handle hObj, FRAMEWIN_Obj *pObj) {
	_RestoreMinimized(hObj, pObj);
	/* When window is not maximized, maximize it */
	if ((pObj->Flags & FRAMEWIN_SF_MAXIMIZED) == 0) {
		WM_HWIN hParent = pObj->Widget.Win.hParent;
		WM_Obj *pParent = WM_H2P(hParent);
		GUI_RECT r = pParent->Rect;
		if (pParent->hParent == 0) {
			r.x1 = LCD_GetXSize();
			r.y1 = LCD_GetYSize();
		}
		pObj->rRestore = pObj->Widget.Win.Rect;
		WM_MoveTo(hObj, r.x0, r.y0);
		WM_SetSize(hObj, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
		FRAMEWIN__UpdatePositions(pObj);
		pObj->Flags |= FRAMEWIN_SF_MAXIMIZED;
		_InvalidateButton(pObj, GUI_ID_MAXIMIZE);
	}
}
void FRAMEWIN_Minimize(FRAMEWIN_Handle hObj) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		_MinimizeFramewin(hObj, pObj);
	}
}
void FRAMEWIN_Maximize(FRAMEWIN_Handle hObj) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		_MaximizeFramewin(hObj, pObj);
	}
}
void FRAMEWIN_Restore(FRAMEWIN_Handle hObj) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		_RestoreMinimized(hObj, pObj);
		_RestoreMaximized(hObj, pObj);
	}
}

void FRAMEWIN_SetBorderSize(FRAMEWIN_Handle hObj, unsigned Size) {
	if (hObj) {
		GUI_RECT r;
		WM_Obj *pChild;
		int Diff, OldSize, OldHeight;
		WM_HWIN hChild;
		FRAMEWIN_Obj *pObj = FRAMEWIN_H2P(hObj);
		OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
		OldSize = pObj->Props.BorderSize;
		Diff = Size - OldSize;
		for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = pChild->hNext) {
			pChild = WM_H2P(hChild);
			r = pChild->Rect;
			GUI_MoveRect(&r, -pObj->Widget.Win.Rect.x0, -pObj->Widget.Win.Rect.y0);
			if ((r.y0 == pObj->Props.BorderSize) && ((r.y1 - r.y0 + 1) == OldHeight)) {
				if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
					WM_MoveWindow(hChild, -Diff, Diff);
				}
				else {
					WM_MoveWindow(hChild, Diff, Diff);
				}
			}
		}
		pObj->Props.BorderSize = Size;
		FRAMEWIN__UpdatePositions(pObj);
		FRAMEWIN_Invalidate(hObj);
	}
}

void FRAMEWIN_SetBarColor(FRAMEWIN_Handle hObj, unsigned Index, RGB_COLOR Color) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		if (Index < GUI_COUNTOF(pObj->Props.aBarColor)) {
			pObj->Props.aBarColor[Index] = Color;
			FRAMEWIN_Invalidate(hObj);
		}
	}
}
void FRAMEWIN_SetTextColor(FRAMEWIN_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		int i;
		pObj = FRAMEWIN_H2P(hObj);
		for (i = 0; i < GUI_COUNTOF(pObj->Props.aTextColor); i++) {
			pObj->Props.aTextColor[i] = Color;
		}
		FRAMEWIN_Invalidate(hObj);
	}
}
void FRAMEWIN_SetTextColorEx(FRAMEWIN_Handle hObj, unsigned Index, RGB_COLOR Color) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
			pObj->Props.aTextColor[Index] = Color;
			FRAMEWIN_Invalidate(hObj);
		}
	}
}
void FRAMEWIN_SetClientColor(FRAMEWIN_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		if (pObj->Props.ClientColor != Color) {
			pObj->Props.ClientColor = Color;
			FRAMEWIN_Invalidate(pObj->hClient);
		}
	}
}

void FRAMEWIN_SetFont(FRAMEWIN_Handle hObj, const GUI_FONT GUI_UNI_PTR *pFont) {
	if (hObj) {
		FRAMEWIN_Obj *pObj = FRAMEWIN_H2P(hObj);
		int OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
		pObj->Props.pFont = pFont;
		FRAMEWIN__UpdatePositions(pObj);
		FRAMEWIN__UpdateButtons(pObj, OldHeight);
		FRAMEWIN_Invalidate(hObj);
	}
}

#ifndef   FRAMEWIN_REACT_BORDER
#define FRAMEWIN_REACT_BORDER 3
#endif
#ifndef   FRAMEWIN_MINSIZE_X
#define FRAMEWIN_MINSIZE_X    20
#endif
#ifndef   FRAMEWIN_MINSIZE_Y
#define FRAMEWIN_MINSIZE_Y    20
#endif
#define FRAMEWIN_RESIZE_X     (1<<0)
#define FRAMEWIN_RESIZE_Y     (1<<1)
#define FRAMEWIN_REPOS_X      (1<<2)
#define FRAMEWIN_REPOS_Y      (1<<3)
#define FRAMEWIN_MOUSEOVER    (1<<4)
#define FRAMEWIN_RESIZE       (FRAMEWIN_RESIZE_X | FRAMEWIN_RESIZE_Y | FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y)
static GUI_HOOK _HOOK_Resizeable;
static int      _CaptureX;
static int      _CaptureY;
static int      _CaptureFlags;
#if GUI_SUPPORT_CURSOR
static const GUI_CURSOR GUI_UNI_PTR *_pOldCursor;
#endif
#if GUI_SUPPORT_CURSOR
static GUI_CONST_STORAGE RGB_COLOR _ColorsCursor[] = {
	 0x0000FF,0x000000,0xFFFFFF
};
static GUI_CONST_STORAGE GUI_LOGPALETTE _PalCursor = {
  3,	/* number of entries */
  1, 	/* Has transparency */
  &_ColorsCursor[0]
};
static GUI_CONST_STORAGE unsigned char _acResizeCursorH[] = {
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x40, 0x00, 0x40, 0x00,
  0x01, 0x40, 0x00, 0x50, 0x00,
  0x06, 0x40, 0x00, 0x64, 0x00,
  0x1A, 0x55, 0x55, 0x69, 0x00,
  0x6A, 0xAA, 0xAA, 0xAA, 0x40,
  0x1A, 0x55, 0x55, 0x69, 0x00,
  0x06, 0x40, 0x00, 0x64, 0x00,
  0x01, 0x40, 0x00, 0x50, 0x00,
  0x00, 0x40, 0x00, 0x40, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00
};
static GUI_CONST_STORAGE GUI_BITMAP _bmResizeCursorH = {
 17,  /* XSize */
 17,  /* YSize */
 5,   /* BytesPerLine */
 2,   /* BitsPerPixel */
 _acResizeCursorH,    /* Pointer to picture data (indices) */
 &_PalCursor          /* Pointer to palette */
};
static GUI_CONST_STORAGE GUI_CURSOR _ResizeCursorH = {
  &_bmResizeCursorH, 8, 8
};
static GUI_CONST_STORAGE unsigned char _acResizeCursorV[] = {
  0x00, 0x00, 0x40, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x06, 0xA4, 0x00, 0x00,
  0x00, 0x1A, 0xA9, 0x00, 0x00,
  0x00, 0x55, 0x95, 0x40, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x55, 0x95, 0x40, 0x00,
  0x00, 0x1A, 0xA9, 0x00, 0x00,
  0x00, 0x06, 0xA4, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x00, 0x40, 0x00, 0x00
};
static GUI_CONST_STORAGE GUI_BITMAP _bmResizeCursorV = {
 17,  /* XSize */
 17,  /* YSize */
 5,   /* BytesPerLine */
 2,   /* BitsPerPixel */
 _acResizeCursorV,    /* Pointer to picture data (indices) */
 &_PalCursor          /* Pointer to palette */
};
static GUI_CONST_STORAGE GUI_CURSOR _ResizeCursorV = {
  &_bmResizeCursorV, 8, 8
};
static GUI_CONST_STORAGE unsigned char _acResizeCursorDD[] = {
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x05, 0x55, 0x00, 0x00, 0x00,
  0x06, 0xA4, 0x00, 0x00, 0x00,
  0x06, 0x90, 0x00, 0x00, 0x00,
  0x06, 0x64, 0x00, 0x00, 0x00,
  0x05, 0x19, 0x00, 0x00, 0x00,
  0x04, 0x06, 0x40, 0x00, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x00, 0x00, 0x64, 0x04, 0x00,
  0x00, 0x00, 0x19, 0x14, 0x00,
  0x00, 0x00, 0x06, 0x64, 0x00,
  0x00, 0x00, 0x01, 0xA4, 0x00,
  0x00, 0x00, 0x06, 0xA4, 0x00,
  0x00, 0x00, 0x15, 0x54, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00
};
static GUI_CONST_STORAGE GUI_BITMAP _bmResizeCursorDD = {
 17,  /* XSize */
 17,  /* YSize */
 5,   /* BytesPerLine */
 2,   /* BitsPerPixel */
 _acResizeCursorDD,   /* Pointer to picture data (indices) */
 &_PalCursor          /* Pointer to palette */
};
static GUI_CONST_STORAGE GUI_CURSOR _ResizeCursorDD = {
  &_bmResizeCursorDD, 8, 8
};
static GUI_CONST_STORAGE unsigned char _acResizeCursorDU[] = {
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x15, 0x54, 0x00,
  0x00, 0x00, 0x06, 0xA4, 0x00,
  0x00, 0x00, 0x01, 0xA4, 0x00,
  0x00, 0x00, 0x06, 0x64, 0x00,
  0x00, 0x00, 0x19, 0x14, 0x00,
  0x00, 0x00, 0x64, 0x04, 0x00,
  0x00, 0x01, 0x90, 0x00, 0x00,
  0x04, 0x06, 0x40, 0x00, 0x00,
  0x05, 0x19, 0x00, 0x00, 0x00,
  0x06, 0x64, 0x00, 0x00, 0x00,
  0x06, 0x90, 0x00, 0x00, 0x00,
  0x06, 0xA4, 0x00, 0x00, 0x00,
  0x05, 0x55, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00
};
static GUI_CONST_STORAGE GUI_BITMAP _bmResizeCursorDU = {
 17,  /* XSize */
 17,  /* YSize */
 5,   /* BytesPerLine */
 2,   /* BitsPerPixel */
 _acResizeCursorDU,   /* Pointer to picture data (indices) */
 &_PalCursor          /* Pointer to palette */
};
static GUI_CONST_STORAGE GUI_CURSOR _ResizeCursorDU = {
  &_bmResizeCursorDU, 8, 8
};
#endif
#if GUI_SUPPORT_CURSOR
static void _SetResizeCursor(int Mode) {
	const GUI_CURSOR GUI_UNI_PTR *pNewCursor = NULL;
	if (Mode) {
		int Direction;
		Direction = Mode & (FRAMEWIN_RESIZE_X | FRAMEWIN_RESIZE_Y);
		if (Direction == FRAMEWIN_RESIZE_X) {
			pNewCursor = &_ResizeCursorH;
		}
		else if (Direction == FRAMEWIN_RESIZE_Y) {
			pNewCursor = &_ResizeCursorV;
		}
		else {
			Direction = Mode & (FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y);
			if ((Direction == (FRAMEWIN_REPOS_X | FRAMEWIN_REPOS_Y)) || !Direction) {
				pNewCursor = &_ResizeCursorDD;
			}
			else {
				pNewCursor = &_ResizeCursorDU;
			}
		}
	}
	if (pNewCursor) {
		const GUI_CURSOR GUI_UNI_PTR *pOldCursor;
		pOldCursor = GUI_CURSOR_Select(pNewCursor);
		if (_pOldCursor == NULL) {
			_pOldCursor = pOldCursor;
		}
	}
	else if (_pOldCursor) {
		GUI_CURSOR_Select(_pOldCursor);
		_pOldCursor = NULL;
	}
}
#endif
static void _SetCapture(FRAMEWIN_Handle hWin, int x, int y, int Mode) {
	if ((_CaptureFlags & FRAMEWIN_REPOS_X) == 0) {
		_CaptureX = x;
	}
	if ((_CaptureFlags & FRAMEWIN_REPOS_Y) == 0) {
		_CaptureY = y;
	}
	if (Mode) {
		if (WM_HasCaptured(hWin) == 0) {
			WM_SetCapture(hWin, 0);
		}
#if GUI_SUPPORT_CURSOR
		_SetResizeCursor(Mode);
#endif
		if (Mode & FRAMEWIN_MOUSEOVER) {
			Mode = 0;
		}
		_CaptureFlags = (Mode | FRAMEWIN_MOUSEOVER);
	}
}
static void _ChangeWindowPosSize(FRAMEWIN_Handle hWin, int *px, int *py) {
	int dx = 0, dy = 0;
	GUI_RECT Rect;
	WM_GetClientRectEx(hWin, &Rect);
	/* Calculate new size of window */
	if (_CaptureFlags & FRAMEWIN_RESIZE_X) {
		dx = (_CaptureFlags & FRAMEWIN_REPOS_X) ? (_CaptureX - *px) : (*px - _CaptureX);
	}
	if (_CaptureFlags & FRAMEWIN_RESIZE_Y) {
		dy = (_CaptureFlags & FRAMEWIN_REPOS_Y) ? (_CaptureY - *py) : (*py - _CaptureY);
	}
	/* Check the minimal size of window */
	if ((Rect.x1 + dx + 1) < FRAMEWIN_MINSIZE_X) {
		dx = FRAMEWIN_MINSIZE_X - Rect.x1 - 1;
		*px = _CaptureX + dx;
	}
	if ((Rect.y1 + dy + 1) < FRAMEWIN_MINSIZE_Y) {
		dy = FRAMEWIN_MINSIZE_Y - Rect.y1 - 1;
		*py = _CaptureY + dy;
	}
	/* Set new window position */
	if (_CaptureFlags & FRAMEWIN_REPOS_X) {
		WM_MoveWindow(hWin, -dx, 0);
	}
	if (_CaptureFlags & FRAMEWIN_REPOS_Y) {
		WM_MoveWindow(hWin, 0, -dy);
	}
	/* Set new window size */
	WM_ResizeWindow(hWin, dx, dy);
}
static int _CheckBorderX(int x, int x1, int Border) {
	int Mode = 0;
	if (x > (x1 - Border)) {
		Mode = FRAMEWIN_RESIZE_X;
	}
	else if (x < (Border)) {
		Mode = FRAMEWIN_RESIZE_X | FRAMEWIN_REPOS_X;
	}
	return Mode;
}
static int _CheckBorderY(int y, int y1, int Border) {
	int Mode = 0;
	if (y > (y1 - Border)) {
		Mode = FRAMEWIN_RESIZE_Y;
	}
	else if (y < (Border)) {
		Mode = FRAMEWIN_RESIZE_Y | FRAMEWIN_REPOS_Y;
	}
	return Mode;
}
static int _CheckReactBorder(FRAMEWIN_Handle hWin, int x, int y) {
	int Mode = 0;
	GUI_RECT r;
	WM_GetClientRectEx(hWin, &r);
	if ((x >= 0) && (y >= 0) && (x <= r.x1) && (y <= r.y1)) {
		Mode |= _CheckBorderX(x, r.x1, FRAMEWIN_REACT_BORDER);
		if (Mode) {
			Mode |= _CheckBorderY(y, r.y1, 4 * FRAMEWIN_REACT_BORDER);
		}
		else {
			Mode |= _CheckBorderY(y, r.y1, FRAMEWIN_REACT_BORDER);
			if (Mode) {
				Mode |= _CheckBorderX(x, r.x1, 4 * FRAMEWIN_REACT_BORDER);
			}
		}
	}
	return Mode;
}
static int _OnTouchResize(FRAMEWIN_Handle hWin, WM_MESSAGE *pMsg) {
	const GUI_PID_STATE *pState = (const GUI_PID_STATE *)pMsg->Data.p;
	if (pState) {  /* Something happened in our area (pressed or released) */
		int x, y, Mode;
		x = pState->x;
		y = pState->y;
		Mode = _CheckReactBorder(hWin, x, y);
		if (pState->Pressed == 1) {
			if (_CaptureFlags & FRAMEWIN_RESIZE) {
				_ChangeWindowPosSize(hWin, &x, &y);
				_SetCapture(hWin, x, y, 0);
				return 1;
			}
			else if (Mode) {
				WM_SetFocus(hWin);
				WM_BringToTop(hWin);
				_SetCapture(hWin, x, y, Mode);
				return 1;
			}
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
			else if (_CaptureFlags) {
				WM_ReleaseCapture();
				return 1;
			}
#endif
		}
		else if (WM_HasCaptured(hWin)) {
			_CaptureFlags &= ~(FRAMEWIN_RESIZE);
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
			if (!Mode)
#endif
			{
				WM_ReleaseCapture();
			}
			return 1;
		}
	}
	return 0;
}
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
static int _ForwardMouseOverMsg(FRAMEWIN_Handle hWin, WM_MESSAGE *pMsg) {
	GUI_PID_STATE *pState = (GUI_PID_STATE *)pMsg->Data.p;
	WM_HWIN hBelow;
	pState->x += WM_GetWindowOrgX(hWin);
	pState->y += WM_GetWindowOrgY(hWin);
	hBelow = WM_Screen2hWin(pState->x, pState->y);
	if (hBelow && (hBelow != hWin)) {
		pState->x -= WM_GetWindowOrgX(hBelow);
		pState->y -= WM_GetWindowOrgY(hBelow);
		WM__SendMessage(hBelow, pMsg);
		return 1;
	}
	return 0;
}
#endif
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
static int _OnMouseOver(FRAMEWIN_Handle hWin, WM_MESSAGE *pMsg) {
	const GUI_PID_STATE *pState = (const GUI_PID_STATE *)pMsg->Data.p;
	if (pState) {
		int x, y, Mode;
		x = pState->x;
		y = pState->y;
		Mode = _CheckReactBorder(hWin, x, y);
		if (Mode) {
			if (_ForwardMouseOverMsg(hWin, pMsg) == 0) {
				_SetCapture(hWin, x, y, Mode | FRAMEWIN_MOUSEOVER);
			}
			return 1;
		}
		else if (WM_HasCaptured(hWin)) {
			if ((_CaptureFlags & FRAMEWIN_RESIZE) == 0) {
				WM_ReleaseCapture();
				_ForwardMouseOverMsg(hWin, pMsg);
			}
			return 1;
		}
	}
	return 0;
}
#endif
static int _HOOKFUNC_Resizeable(WM_MESSAGE *pMsg) {
	WM_HWIN hWin = pMsg->hWin;
	if (WM_HasCaptured(hWin) && (_CaptureFlags == 0)) {
		return 0;
	}
	if (FRAMEWIN_IsMinimized(hWin) || FRAMEWIN_IsMaximized(hWin)) {
		return 0;
	}
	switch (pMsg->MsgId) {
		case WM_TOUCH:
			return _OnTouchResize(hWin, pMsg);
#if (GUI_SUPPORT_MOUSE & GUI_SUPPORT_CURSOR)
		case WM_MOUSEOVER:
			return _OnMouseOver(hWin, pMsg);
#endif
		case WM_CAPTURE_RELEASED:
#if GUI_SUPPORT_CURSOR
			_SetResizeCursor(0);
#endif
			_CaptureFlags = 0;
			return 1;
	}
	return 0;
}
void FRAMEWIN_SetResizeable(FRAMEWIN_Handle hObj, int State) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		pObj = FRAMEWIN_H2P(hObj);
		if (pObj) {
			if (State) {
				GUI_HOOK_Add(&pObj->pFirstHook, &_HOOK_Resizeable, &_HOOKFUNC_Resizeable);
			}
			else {
				GUI_HOOK_Remove(&pObj->pFirstHook, &_HOOK_Resizeable);
			}
		}
	}
}


int FRAMEWIN_SetTitleHeight(FRAMEWIN_Handle hObj, int Height) {
	int r = 0;
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		int OldHeight;
		pObj = FRAMEWIN_H2P(hObj);
		r = pObj->Props.TitleHeight;
		if (Height != r) {
			OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
			pObj->Props.TitleHeight = Height;
			FRAMEWIN__UpdatePositions(pObj);
			FRAMEWIN__UpdateButtons(pObj, OldHeight);
			FRAMEWIN_Invalidate(hObj);
		}
	}
	return r;
}

static void _ShowHideButtons(FRAMEWIN_Obj *pObj) {
	WM_HWIN hChild;
	WM_Obj *pChild;
	int y0;
	for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = pChild->hNext) {
		pChild = WM_H2P(hChild);
		y0 = pChild->Rect.y0 - pObj->Widget.Win.Rect.y0;
		if ((y0 == pObj->Props.BorderSize) && (hChild != pObj->hClient)) {
			if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
				WM_ShowWindow(hChild);
			}
			else {
				WM_HideWindow(hChild);
			}
		}
	}
}
void FRAMEWIN_SetTitleVis(FRAMEWIN_Handle hObj, int Show) {
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		int State;
		pObj = FRAMEWIN_H2P(hObj);
		State = pObj->Widget.State;
		if (Show) {
			State |= FRAMEWIN_SF_TITLEVIS;
		}
		else {
			State &= ~FRAMEWIN_SF_TITLEVIS;
		}
		if (pObj->Widget.State != State) {
			pObj->Widget.State = State;
			FRAMEWIN__UpdatePositions(pObj);
			_ShowHideButtons(pObj);
			if (pObj->Flags & FRAMEWIN_SF_MINIMIZED) {
				if (State & FRAMEWIN_SF_TITLEVIS) {
					WM_ShowWindow(hObj);
				}
				else {
					WM_HideWindow(hObj);
				}
			}
			FRAMEWIN_Invalidate(hObj);
		}
	}
}

/*********************************************************************
*
*       FRAMEWIN__UpdateButtons
*
* Purpose:
*   Adjust button positions & size.
*   This is required after resizing the title bar
*/
void FRAMEWIN__UpdateButtons(FRAMEWIN_Obj *pObj, int OldHeight) {
	int TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
	int Diff = TitleHeight - OldHeight;
	if (Diff) {
		WM_HWIN hLeft;
		WM_HWIN hRight;
		WM_HWIN hChild;
		WM_Obj *pChild;
		GUI_RECT r;
		int xLeft, xRight, n;
		n = 0;
		do {
			hLeft = hRight = 0;
			xLeft = GUI_XMAX;
			xRight = GUI_XMIN;
			for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = pChild->hNext) {
				pChild = WM_H2P(hChild);
				r = pChild->Rect;
				GUI_MoveRect(&r, -pObj->Widget.Win.Rect.x0, -pObj->Widget.Win.Rect.y0);
				if ((r.y0 == pObj->Props.BorderSize) && ((r.y1 - r.y0 + 1) == OldHeight)) {
					if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
						if (r.x1 > xRight) {
							hRight = hChild;
							xRight = r.x0;
						}
					}
					else {
						if (r.x0 < xLeft) {
							hLeft = hChild;
							xLeft = r.x0;
						}
					}
				}
			}
			if (hLeft) {
				WM_ResizeWindow(hLeft, Diff, Diff);
				WM_MoveWindow(hLeft, n * Diff, 0);
			}
			if (hRight) {
				WM_ResizeWindow(hRight, Diff, Diff);
				WM_MoveWindow(hRight, -(n * Diff), 0);
			}
			n++;
		} while (hLeft || hRight);
	}
}

WM_HWIN FRAMEWIN_AddButton(FRAMEWIN_Handle hObj, int Flags, int Off, int Id) {
	WM_HWIN r = 0;
	if (hObj) {
		FRAMEWIN_Obj *pObj;
		POSITIONS Pos;
		int Size = FRAMEWIN_GetTitleHeight(hObj);
		int BorderSize = FRAMEWIN_GetBorderSize(hObj);
		int WinFlags, x;

		pObj = FRAMEWIN_H2P(hObj);
		FRAMEWIN__CalcPositions(pObj, &Pos);
		if (Flags & FRAMEWIN_BUTTON_RIGHT) {
			x = Pos.rTitleText.x1 - (Size - 1) - Off;
			WinFlags = WM_CF_SHOW | WM_CF_ANCHOR_RIGHT;
		}
		else {
			x = Pos.rTitleText.x0 + Off;
			WinFlags = WM_CF_SHOW;
		}
		r = BUTTON_CreateAsChild(x, BorderSize, Size, Size, hObj, Id, WinFlags);
		BUTTON_SetFocussable(r, 0);

	}
	return r;
}

/*********************************************************************
*
*       Callback
*
* This is the overwritten callback routine for the button.
* The primary reason for overwriting it is that we define the default
* action of the Framewindow here.
* It works as follows:
* - User clicks and releases the button
*   -> BUTTON sends WM_NOTIFY_PARENT to FRAMEWIN
*     -> FRAMEWIN either a) reacts or b)sends WM_NOTIFY_PARENT_REFLECTION back
*       In case of a) This module reacts !
*/
static void _cbClose(WM_MESSAGE *pMsg) {
	if (pMsg->MsgId == WM_NOTIFY_PARENT_REFLECTION) {
		WM_DeleteWindow(pMsg->hWinSrc);
		return;                                       /* We are done ! */
	}
	BUTTON_Callback(pMsg);
}
static void _DrawClose(void) {
	GUI_RECT r;
	int i, Size;
	WM_GetInsideRect(&r);
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	Size = r.x1 - r.x0 - 2;
	WM_ITERATE_START(&r); {
		for (i = 2; i < Size; i++) {
			LCD_DrawHLine(r.x0 + i, r.y0 + i, r.x0 + i + 1);
			LCD_DrawHLine(r.x1 - i - 1, r.y0 + i, r.x1 - i);
		}
	} WM_ITERATE_END();
}
WM_HWIN FRAMEWIN_AddCloseButton(FRAMEWIN_Handle hObj, int Flags, int Off) {
	WM_HWIN hButton;
	hButton = FRAMEWIN_AddButton(hObj, Flags, Off, GUI_ID_CLOSE);
	BUTTON_SetSelfDraw(hButton, 0, &_DrawClose);
	WM_SetCallback(hButton, _cbClose);
	return hButton;
}
/*********************************************************************
*
*       Callback
*
* This is the overwritten callback routine for the button.
* The primary reason for overwriting it is that we define the default
* action of the Framewindow here.
* It works as follows:
* - User clicks and releases the button
*   -> BUTTON sends WM_NOTIFY_PARENT to FRAMEWIN
*     -> FRAMEWIN either a) reacts or b)sends WM_NOTIFY_PARENT_REFLECTION back
*       In case of a) This module reacts !
*/
static void _cbMax(WM_MESSAGE *pMsg) {
	if (pMsg->MsgId == WM_NOTIFY_PARENT_REFLECTION) {
		WM_HWIN hWin = pMsg->hWinSrc;
		FRAMEWIN_Obj *pObj = FRAMEWIN_H2P(hWin);
		if (pObj->Flags & FRAMEWIN_SF_MAXIMIZED) {
			FRAMEWIN_Restore(hWin);
		}
		else {
			FRAMEWIN_Maximize(hWin);
		}
		return;                                       /* We are done ! */
	}
	BUTTON_Callback(pMsg);
}
static void _PaintMax(void) {
	GUI_RECT r;
	WM_GetInsideRect(&r);
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	WM_ITERATE_START(&r); {
		LCD_DrawHLine(r.x0 + 1, r.y0 + 1, r.x1 - 1);
		LCD_DrawHLine(r.x0 + 1, r.y0 + 2, r.x1 - 1);
		LCD_DrawHLine(r.x0 + 1, r.y1 - 1, r.x1 - 1);
		LCD_DrawVLine(r.x0 + 1, r.y0 + 1, r.y1 - 1);
		LCD_DrawVLine(r.x1 - 1, r.y0 + 1, r.y1 - 1);
	} WM_ITERATE_END();
}
static void _DrawRestoreClose(void) {
	GUI_RECT r;
	int Size;
	WM_GetInsideRect(&r);
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	Size = ((r.x1 - r.x0 + 1) << 1) / 3;
	WM_ITERATE_START(&r); {
		LCD_DrawHLine(r.x1 - Size, r.y0 + 1, r.x1 - 1);
		LCD_DrawHLine(r.x1 - Size, r.y0 + 2, r.x1 - 1);
		LCD_DrawHLine(r.x0 + Size, r.y0 + Size, r.x1 - 1);
		LCD_DrawVLine(r.x1 - Size, r.y0 + 1, r.y1 - Size);
		LCD_DrawVLine(r.x1 - 1, r.y0 + 1, r.y0 + Size);
		LCD_DrawHLine(r.x0 + 1, r.y1 - Size, r.x0 + Size);
		LCD_DrawHLine(r.x0 + 1, r.y1 - Size + 1, r.x0 + Size);
		LCD_DrawHLine(r.x0 + 1, r.y1 - 1, r.x0 + Size);
		LCD_DrawVLine(r.x0 + 1, r.y1 - Size, r.y1 - 1);
		LCD_DrawVLine(r.x0 + Size, r.y1 - Size, r.y1 - 1);
	} WM_ITERATE_END();
}
static void _DrawMax(void) {
	FRAMEWIN_Obj *pObj;
	WM_HWIN hWin;
	hWin = WM_GetActiveWindow();
	hWin = WM_GetParent(hWin);
	pObj = FRAMEWIN_H2P(hWin);
	if (pObj->Flags & FRAMEWIN_SF_MAXIMIZED) {
		_DrawRestoreClose();
	}
	else {
		_PaintMax();
	}
}
WM_HWIN FRAMEWIN_AddMaxButton(FRAMEWIN_Handle hObj, int Flags, int Off) {
	WM_HWIN hButton;
	hButton = FRAMEWIN_AddButton(hObj, Flags, Off, GUI_ID_MAXIMIZE);
	BUTTON_SetSelfDraw(hButton, 0, &_DrawMax);
	WM_SetCallback(hButton, _cbMax);
	return hButton;
}

/*********************************************************************
*
*       Callback
*
* This is the overwritten callback routine for the button.
* The primary reason for overwriting it is that we define the default
* action of the Framewindow here.
* It works as follows:
* - User clicks and releases the button
*   -> BUTTON sends WM_NOTIFY_PARENT to FRAMEWIN
*     -> FRAMEWIN either a) reacts or b)sends WM_NOTIFY_PARENT_REFLECTION back
*       In case of a) This module reacts !
*/
static void _cbMin(WM_MESSAGE *pMsg) {
	if (pMsg->MsgId == WM_NOTIFY_PARENT_REFLECTION) {
		WM_HWIN hWin = pMsg->hWinSrc;
		FRAMEWIN_Obj *pObj = FRAMEWIN_H2P(hWin);
		if (pObj->Flags & FRAMEWIN_SF_MINIMIZED) {
			FRAMEWIN_Restore(hWin);
		}
		else {
			FRAMEWIN_Minimize(hWin);
		}
		return;                                       /* We are done ! */
	}
	BUTTON_Callback(pMsg);
}
static void _PaintMin(void) {
	GUI_RECT r;
	int i, Size;
	WM_GetInsideRect(&r);
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	Size = (r.x1 - r.x0 + 1) >> 1;
	WM_ITERATE_START(&r); {
		for (i = 1; i < Size; i++) {
			LCD_DrawHLine(r.x0 + i, r.y1 - i - (Size >> 1), r.x1 - i);
		}
	} WM_ITERATE_END();
}
static void _DrawRestoreMin(void) {
	GUI_RECT r;
	int i, Size;
	WM_GetInsideRect(&r);
	WM_ADDORG(r.x0, r.y0);
	WM_ADDORG(r.x1, r.y1);
	Size = (r.x1 - r.x0 + 1) >> 1;
	WM_ITERATE_START(&r); {
		for (i = 1; i < Size; i++) {
			LCD_DrawHLine(r.x0 + i, r.y0 + i + (Size >> 1), r.x1 - i);
		}
	} WM_ITERATE_END();
}
static void _DrawMin(void) {
	FRAMEWIN_Obj *pObj;
	WM_HWIN hWin;
	hWin = WM_GetActiveWindow();
	hWin = WM_GetParent(hWin);
	pObj = FRAMEWIN_H2P(hWin);
	if (pObj->Flags & FRAMEWIN_SF_MINIMIZED) {
		_DrawRestoreMin();
	}
	else {
		_PaintMin();
	}
}
WM_HWIN FRAMEWIN_AddMinButton(FRAMEWIN_Handle hObj, int Flags, int Off) {
	WM_HWIN hButton;
	hButton = FRAMEWIN_AddButton(hObj, Flags, Off, GUI_ID_MINIMIZE);
	BUTTON_SetSelfDraw(hButton, 0, &_DrawMin);
	WM_SetCallback(hButton, _cbMin);
	return hButton;
}
