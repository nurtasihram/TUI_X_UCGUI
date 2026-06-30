#include "GUIDebug.h"
#include "GUI_Protected.h"
#include "GUI_ARRAY.h"

#include "WM_Intern.h"

#include "HEADER.h"
#include "SCROLLBAR.h"

#include "LISTVIEW.h"
#include "LISTVIEW_Private.h"

/* Define default fonts */
#define LISTVIEW_FONT_DEFAULT &GUI_Font13_1
/* Define colors */
#define LISTVIEW_BKCOLOR0_DEFAULT RGB_WHITE     /* Not selected */
#define LISTVIEW_BKCOLOR1_DEFAULT RGB_GRAY      /* Selected, no focus */
#define LISTVIEW_BKCOLOR2_DEFAULT RGB_BLUE      /* Selected, focus */
#define LISTVIEW_TEXTCOLOR0_DEFAULT RGB_BLACK   /* Not selected */
#define LISTVIEW_TEXTCOLOR1_DEFAULT RGB_WHITE   /* Selected, no focus */
#define LISTVIEW_TEXTCOLOR2_DEFAULT RGB_WHITE   /* Selected, focus */
#define LISTVIEW_GRIDCOLOR_DEFAULT RGB_LIGHTGRAY
/* Define default alignment */
#define LISTVIEW_ALIGN_DEFAULT (GUI_TA_VCENTER | GUI_TA_HCENTER)
LISTVIEW_PROPS LISTVIEW_DefaultProps = {
  LISTVIEW_BKCOLOR0_DEFAULT,
  LISTVIEW_BKCOLOR1_DEFAULT,
  LISTVIEW_BKCOLOR2_DEFAULT,
  LISTVIEW_TEXTCOLOR0_DEFAULT,
  LISTVIEW_TEXTCOLOR1_DEFAULT,
  LISTVIEW_TEXTCOLOR2_DEFAULT,
  LISTVIEW_GRIDCOLOR_DEFAULT,
  LISTVIEW_FONT_DEFAULT
};
unsigned LISTVIEW__GetRowDistY(LISTVIEW_Obj *pObj) {
	unsigned RowDistY;
	if (pObj->RowDistY) {
		RowDistY = pObj->RowDistY;
	}
	else {
		RowDistY = GUI_GetYDistOfFont(pObj->Props.pFont);
		if (pObj->ShowGrid) {
			RowDistY++;
		}
	}
	return RowDistY;
}
/*********************************************************************
*
*       _GetNumVisibleRows
*
* Purpose:
*   Returns the number of visible rows according the header
*   and (if exist) horizontal scrollbar.
*
* Return value:
*   Number of visible rows. If no entire row can be displayed, this
*   function will return one.
*/
static unsigned _GetNumVisibleRows(LISTVIEW_Obj *pObj) {
	unsigned RowDistY, ySize, r = 1;
	GUI_RECT Rect;
	WM_GetInsideRectExScrollbar(pObj, &Rect);
	ySize = Rect.y1 - Rect.y0 + 1 - HEADER_GetHeight(pObj->hHeader);
	RowDistY = LISTVIEW__GetRowDistY(pObj);
	if (RowDistY) {
		r = ySize / RowDistY;
		r = (r == 0) ? 1 : r;
	}
	return r;
}
static void _OnPaint(LISTVIEW_Obj *pObj, const GUI_RECT *pClipRect) {
	const GUI_ARRAY *pRow;
	GUI_RECT ClipRect, Rect;
	int NumRows, NumVisRows, NumColumns;
	int LBorder, RBorder, EffectSize;
	int xPos, yPos, Width, RowDistY;
	int Align, i, j, EndRow;
	/* Init some values */
	NumColumns = HEADER_GetNumItems(pObj->hHeader);
	NumRows = GUI_ARRAY_GetNumItems(&pObj->RowArray);
	NumVisRows = _GetNumVisibleRows(pObj);
	RowDistY = LISTVIEW__GetRowDistY(pObj);
	LBorder = pObj->LBorder;
	RBorder = pObj->RBorder;
	EffectSize = pObj->Widget.pEffect->EffectSize;
	yPos = HEADER_GetHeight(pObj->hHeader) + EffectSize;
	EndRow = pObj->ScrollStateV.v + (((NumVisRows + 1) > NumRows) ? NumRows : NumVisRows + 1);
	/* Calculate clipping rectangle */
	ClipRect = *pClipRect;
	GUI_MoveRect(&ClipRect, -pObj->Widget.Win.Rect.x0, -pObj->Widget.Win.Rect.y0);
	WM_GetInsideRectExScrollbar(pObj, &Rect);
	GUI__IntersectRect(&ClipRect, &Rect);
	/* Set drawing color, font and text mode */
	GUI_SetColor(pObj->Props.aTextColor[0]);
	GUI_SetFont(pObj->Props.pFont);
	GUI_SetTextMode(DRAWMODE_TRANS);
	/* Do the drawing */
	for (i = pObj->ScrollStateV.v; i < EndRow; i++) {
		pRow = (const GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, i);
		if (pRow) {
			Rect.y0 = yPos;
			/* Break when all other rows are outside the drawing area */
			if (Rect.y0 > ClipRect.y1) {
				break;
			}
			Rect.y1 = yPos + RowDistY - 1;
			/* Make sure that we draw only when row is in drawing area */
			if (Rect.y1 >= ClipRect.y0) {
				int ColorIndex;
				/* Set background color */
				if (i == pObj->Sel) {
					ColorIndex = (pObj->Widget.State & WIDGET_STATE_FOCUS) ? 2 : 1;
				}
				else {
					ColorIndex = 0;
				}
				GUI_SetBkColor(pObj->Props.aBkColor[ColorIndex]);
				/* Iterate over all columns */
				if (pObj->ShowGrid) {
					Rect.y1--;
				}
				xPos = EffectSize - pObj->ScrollStateH.v;
				for (j = 0; j < NumColumns; j++) {
					Width = HEADER_GetItemWidth(pObj->hHeader, j);
					Rect.x0 = xPos;
					/* Break when all other columns are outside the drawing area */
					if (Rect.x0 > ClipRect.x1) {
						break;
					}
					Rect.x1 = xPos + Width - 1;
					/* Make sure that we draw only when column is in drawing area */
					if (Rect.x1 >= ClipRect.x0) {
						LISTVIEW_ITEM *pItem;
						pItem = (LISTVIEW_ITEM *)GUI_ARRAY_GetpItem(pRow, j);
						if (pItem->hItemInfo) {
							LISTVIEW_ITEM_INFO *pItemInfo;
							pItemInfo = (LISTVIEW_ITEM_INFO *)(pItem->hItemInfo);
							GUI_SetBkColor(pItemInfo->aBkColor[ColorIndex]);
							GUI_SetColor(pItemInfo->aTextColor[ColorIndex]);
						}
						else {
							GUI_SetColor(pObj->Props.aTextColor[ColorIndex]);
						}
						/* Clear background */
						GUI_ClearRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1);
						/* Draw text */
						Rect.x0 += LBorder;
						Rect.x1 -= RBorder;
						Align = *((int *)GUI_ARRAY_GetpItem(&pObj->AlignArray, j));
						GUI_DispStringInRect(pItem->acText, &Rect, Align);
						if (pItem->hItemInfo) {
							GUI_SetBkColor(pObj->Props.aBkColor[ColorIndex]);
						}
					}
					xPos += Width;
				}
				/* Clear unused area to the right of items */
				if (xPos <= ClipRect.x1) {
					GUI_ClearRect(xPos, Rect.y0, ClipRect.x1, Rect.y1);
				}
			}
			yPos += RowDistY;
		}
	}
	/* Clear unused area below items */
	if (yPos <= ClipRect.y1) {
		GUI_SetBkColor(pObj->Props.aBkColor[0]);
		GUI_ClearRect(ClipRect.x0, yPos, ClipRect.x1, ClipRect.y1);
	}
	/* Draw grid */
	if (pObj->ShowGrid) {
		GUI_SetColor(pObj->Props.GridColor);
		yPos = HEADER_GetHeight(pObj->hHeader) + EffectSize - 1;
		for (i = 0; i < NumVisRows; i++) {
			yPos += RowDistY;
			/* Break when all other rows are outside the drawing area */
			if (yPos > ClipRect.y1) {
				break;
			}
			/* Make sure that we draw only when row is in drawing area */
			if (yPos >= ClipRect.y0) {
				GUI_DrawHLine(yPos, ClipRect.x0, ClipRect.x1);
			}
		}
		xPos = EffectSize - pObj->ScrollStateH.v;
		for (i = 0; i < NumColumns; i++) {
			xPos += HEADER_GetItemWidth(pObj->hHeader, i);
			/* Break when all other columns are outside the drawing area */
			if (xPos > ClipRect.x1) {
				break;
			}
			/* Make sure that we draw only when column is in drawing area */
			if (xPos >= ClipRect.x0) {
				GUI_DrawVLine(xPos, ClipRect.y0, ClipRect.y1);
			}
		}
	}
	/* Draw the effect */
	WIDGET__EFFECT_DrawDown(&pObj->Widget);
}
void LISTVIEW__InvalidateInsideArea(LISTVIEW_Obj *pObj) {
	GUI_RECT Rect;
	int HeaderHeight;
	HeaderHeight = HEADER_GetHeight(pObj->hHeader);
	WM_GetInsideRectExScrollbar(pObj, &Rect);
	Rect.y0 += HeaderHeight;
	WM_InvalidateRect(pObj, &Rect);
}
void LISTVIEW__InvalidateRow(LISTVIEW_Obj *pObj, int Sel) {
	if (Sel >= 0) {
		GUI_RECT Rect;
		int HeaderHeight, RowDistY;
		HeaderHeight = HEADER_GetHeight(pObj->hHeader);
		RowDistY = LISTVIEW__GetRowDistY(pObj);
		WM_GetInsideRectExScrollbar(pObj, &Rect);
		Rect.y0 += HeaderHeight + (Sel - pObj->ScrollStateV.v) * RowDistY;
		Rect.y1 = Rect.y0 + RowDistY - 1;
		WM_InvalidateRect(pObj, &Rect);
	}
}
static void _SetSelFromPos(LISTVIEW_Obj *pObj, const GUI_PID_STATE *pState) {
	GUI_RECT Rect;
	int x, y, HeaderHeight;
	HeaderHeight = HEADER_GetHeight(pObj->hHeader);
	WM_GetInsideRectExScrollbar(pObj, &Rect);
	x = pState->x - Rect.x0;
	y = pState->y - Rect.y0 - HeaderHeight;
	Rect.x1 -= Rect.x0;
	Rect.y1 -= Rect.y0;
	if ((x >= 0) && (x <= Rect.x1) && (y >= 0) && (y <= (Rect.y1 - HeaderHeight))) {
		unsigned Sel;
		Sel = (y / LISTVIEW__GetRowDistY(pObj)) + pObj->ScrollStateV.v;
		if (Sel < GUI_ARRAY_GetNumItems(&pObj->RowArray)) {
			LISTVIEW_SetSel(pObj, Sel);
		}
	}
}
/*********************************************************************
*
*       _NotifyOwner
*
* Purpose:
*   Notify owner of the window.
*   If no owner is registered, the parent is considered owner.
*/
static void _NotifyOwner(WM_HWIN hObj, int Notification) {
	LISTVIEW_Obj *pObj = (hObj);
	WM_HWIN hOwner = pObj->hOwner ? pObj->hOwner : WM_GetParent(hObj);
	WM_MESSAGE Msg = { 0 };
	Msg.MsgId = WM_NOTIFY_PARENT;
	Msg.Data = Notification;
	WM_SendMessage(hOwner, &Msg);
}
static void _OnTouch(LISTVIEW_Obj *pObj, const GUI_PID_STATE *pState) {
	int Notification;
	GUI_USE_PARA(pObj);
	if (pState) {  /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			_SetSelFromPos(pObj, pState);
			Notification = WM_NOTIFICATION_CLICKED;
			WM_SetFocus(pObj);
		}
		else
			Notification = WM_NOTIFICATION_RELEASED;
	}
	else
		Notification = WM_NOTIFICATION_MOVED_OUT;
	_NotifyOwner(pObj, Notification);
}
static int _OnKey(LISTVIEW_Obj *pObj, const WM_KEY_INFO *pInfo) {
	if (pInfo->PressedCnt > 0) {
		int Key = pInfo->Key;
		if (_AddKey(pObj, Key)) 
			return 1; /* Key has been consumed */
	}
	return 0; /* Key has not been consumed */
}
/*********************************************************************
*
*       _GetXSize
*
* Purpose:
*   Returns the width of the inside listview area.
*/
static int _GetXSize(LISTVIEW_Handle hObj) {
	GUI_RECT Rect;
	WM_GetInsideRectExScrollbar(hObj, &Rect);
	return Rect.x1 + 1;
}
/*********************************************************************
*
*       _GetHeaderWidth
*
* Purpose:
*   Returns the width of all items in header.
*
* Return value:
*   NumItems > 0:  width of all items.
*   NumItems = 0:  1 (to avoid problem with horizontal scrollbar)
*/
static int _GetHeaderWidth(LISTVIEW_Obj *pObj, HEADER_Handle hHeader) {
	int NumItems, i, r = 1;
	NumItems = HEADER_GetNumItems(hHeader);
	if (NumItems) {
		for (i = 0, r = 0; i < NumItems; i++) {
			r += HEADER_GetItemWidth(hHeader, i);
		}
	}
	if (pObj->ScrollStateH.v > (r - pObj->ScrollStateH.PageSize)) {
		r += pObj->ScrollStateH.PageSize - (r - pObj->ScrollStateH.v);
	}
	return r;
}
/*********************************************************************
*
*       LISTVIEW__UpdateScrollPos
*
* Purpose:
*   Checks whether if we must scroll up or scroll down to ensure
*   that selection is in the visible area. This function also
*   makes sure that scroll positions are in valid ranges.
*
* Return value:
*   Difference between old and new vertical scroll pos.
*/
int LISTVIEW__UpdateScrollPos(LISTVIEW_Obj *pObj) {
	int PrevScrollStateV;
	PrevScrollStateV = pObj->ScrollStateV.v;
	if (pObj->Sel >= 0) {
		WM_CheckScrollPos(&pObj->ScrollStateV, pObj->Sel, 0, 0);
	}
	else {
		WM_CheckScrollBounds(&pObj->ScrollStateV);
	}
	WM_CheckScrollBounds(&pObj->ScrollStateH);
	WIDGET__SetScrollState(pObj, &pObj->ScrollStateV, &pObj->ScrollStateH);
	return pObj->ScrollStateV.v - PrevScrollStateV;
}
/*********************************************************************
*
*       LISTVIEW__UpdateScrollParas
*
* Purpose:
*   Calculates number of items and page size of both vertical
*   and horizontal scrollbar. After this LISTVIEW__UpdateScrollPos will
*   be called to ensure scroll positions are in valid ranges.
*/
int LISTVIEW__UpdateScrollParas(LISTVIEW_Obj *pObj) {
	int NumRows;
	NumRows = GUI_ARRAY_GetNumItems(&pObj->RowArray);
	/* update vertical scrollbar */
	pObj->ScrollStateV.PageSize = _GetNumVisibleRows(pObj);
	pObj->ScrollStateV.NumItems = (NumRows) ? NumRows : 1;
	/* update horizontal scrollbar */
	pObj->ScrollStateH.PageSize = _GetXSize(pObj);
	pObj->ScrollStateH.NumItems = _GetHeaderWidth(pObj, pObj->hHeader);
	return LISTVIEW__UpdateScrollPos(pObj);
}
/*********************************************************************
*
*       _FreeAttached
*
* Purpose:
*   Delete attached objects (if any).
*/
static void _FreeAttached(LISTVIEW_Obj *pObj) {
	int i, j, NumRows, NumColumns;
	NumRows = GUI_ARRAY_GetNumItems(&pObj->RowArray);
	NumColumns = GUI_ARRAY_GetNumItems(&pObj->AlignArray);
	for (i = 0; i < NumRows; i++) {
		GUI_ARRAY *pRow;
		pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, i);
		/* Delete attached info items */
		for (j = 0; j < NumColumns; j++) {
			LISTVIEW_ITEM *pItem;
			pItem = (LISTVIEW_ITEM *)GUI_ARRAY_GetpItem(pRow, j);
			if (pItem->hItemInfo) {
				GUI_ALLOC_Free(pItem->hItemInfo);
			}
		}
		/* Delete row */
		GUI_ARRAY_Delete(pRow);
	}
	GUI_ARRAY_Delete(&pObj->AlignArray);
	GUI_ARRAY_Delete(&pObj->RowArray);
}
/*********************************************************************
*
*       _AddKey
*
* Returns: 1 if Key has been consumed
*          0 else
*/
static int _AddKey(LISTVIEW_Handle hObj, int Key) {
	switch (Key) {
		case GUI_KEY_DOWN:
			LISTVIEW_IncSel(hObj);
			return 1;               /* Key has been consumed */
		case GUI_KEY_UP:
			LISTVIEW_DecSel(hObj);
			return 1;               /* Key has been consumed */
	}
	return 0;                 /* Key has NOT been consumed */
}
static void _LISTVIEW_Callback(WM_HWIN hWin, WM_MESSAGE *pMsg) {
	LISTVIEW_Obj *pObj = hWin;
	/* Let widget handle the standard messages */
	if (WIDGET_HandleActive(pObj, pMsg) == 0) {
		return;
	}
	switch (pMsg->MsgId) {
		case WM_NOTIFY_CLIENTCHANGE:
		case WM_SIZE:
			LISTVIEW__UpdateScrollParas(pObj);
			return;
		case WM_NOTIFY_PARENT:
			switch (pMsg->Data) {
				case WM_NOTIFICATION_CHILD_DELETED:
					/* make sure we do not send any messages to the header child once it has been deleted */
					if (pMsg->hWinSrc == pObj->hHeader)
						pObj->hHeader = NULL;
					break;
				case WM_NOTIFICATION_VALUE_CHANGED: {
					WM_SCROLL_STATE ScrollState;
					if (pMsg->hWinSrc == WM_GetScrollbarV(pObj)) {
						WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
						pObj->ScrollStateV.v = ScrollState.v;
						LISTVIEW__InvalidateInsideArea(pObj);
						_NotifyOwner(pObj, WM_NOTIFICATION_SCROLL_CHANGED);
					}
					else if (pMsg->hWinSrc == WM_GetScrollbarH(pObj)) {
						WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
						pObj->ScrollStateH.v = ScrollState.v;
						LISTVIEW__UpdateScrollParas(pObj);
						HEADER_SetScrollPos(pObj->hHeader, pObj->ScrollStateH.v);
						_NotifyOwner(pObj, WM_NOTIFICATION_SCROLL_CHANGED);
					}
					break;
				}
				case WM_NOTIFICATION_SCROLLBAR_ADDED:
					LISTVIEW__UpdateScrollParas(pObj);
					break;
			}
			return;
		case WM_PAINT:
			_OnPaint(pObj, (const GUI_RECT *)pMsg->Data);
			return;
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)pMsg->Data);
			return;        /* Important: message handled ! */
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)pMsg->Data))
				return;
			break; /* No return here ... WM_DefaultProc needs to be called */
		case WM_DELETE:
			_FreeAttached(pObj);
			break; /* No return here ... WM_DefaultProc needs to be called */
	}
	WM_DefaultProc(hWin, pMsg);
}
/* Note: the parameters to a create function may vary.
		 Some widgets may have multiple create functions */
LISTVIEW_Handle LISTVIEW_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
								  int WinFlags, int ExFlags, int Id) {
	LISTVIEW_Handle hObj;
	GUI_USE_PARA(ExFlags);
	/* Create the window */
	if ((xsize == 0) && (ysize == 0) && (x0 == 0) && (y0 == 0)) {
		GUI_RECT Rect;
		WM_GetClientRectEx(hParent, &Rect);
		xsize = Rect.x1 - Rect.x0 + 1;
		ysize = Rect.y1 - Rect.y0 + 1;
	}
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, &_LISTVIEW_Callback,
								  sizeof(LISTVIEW_Obj) - sizeof(WM_Obj));
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		/* Init sub-classes */
		GUI_ARRAY_CREATE(&pObj->RowArray);
		GUI_ARRAY_CREATE(&pObj->AlignArray);
		/* Init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
		/* Init member variables */
		pObj->Props = LISTVIEW_DefaultProps;
		pObj->ShowGrid = 0;
		pObj->RowDistY = 0;
		pObj->Sel = -1;
		pObj->LBorder = 1;
		pObj->RBorder = 1;
		pObj->hHeader = HEADER_CreateEx(0, 0, 0, 0, hObj, WM_CF_SHOW, 0, 0);
		LISTVIEW__UpdateScrollParas(pObj);

	}
	else {
	}
	return hObj;
}
void LISTVIEW_IncSel(LISTVIEW_Handle hObj) {
	int Sel = LISTVIEW_GetSel(hObj);
	LISTVIEW_SetSel(hObj, Sel + 1);
}
void LISTVIEW_DecSel(LISTVIEW_Handle hObj) {
	int Sel = LISTVIEW_GetSel(hObj);
	if (Sel) {
		LISTVIEW_SetSel(hObj, Sel - 1);
	}
}
void LISTVIEW_AddColumn(LISTVIEW_Handle hObj, int Width, const char *s, int Align) {
	if (hObj) {
		LISTVIEW_Obj *pObj;
		unsigned NumRows;

		pObj = (hObj);
		HEADER_AddItem(pObj->hHeader, Width, s, Align);   /* Modify header */
		GUI_ARRAY_AddItem(&pObj->AlignArray, &Align, sizeof(int));
		NumRows = LISTVIEW_GetNumRows(hObj);
		if (NumRows) {
			GUI_ARRAY *pRow;
			unsigned i;
			for (i = 0; i < NumRows; i++) {
				pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, i);
				GUI_ARRAY_AddItem(pRow, NULL, sizeof(LISTVIEW_ITEM) + 1);
			}
		}
		LISTVIEW__UpdateScrollParas(pObj);
		LISTVIEW__InvalidateInsideArea(pObj);

	}
}
void LISTVIEW_AddRow(LISTVIEW_Handle hObj, const GUI_ConstString *ppText) {
	if (hObj) {
		LISTVIEW_Obj *pObj;
		int NumRows;

		pObj = (hObj);
		NumRows = GUI_ARRAY_GetNumItems(&pObj->RowArray);
		/* Create GUI_ARRAY for the new row */
		if (GUI_ARRAY_AddItem(&pObj->RowArray, NULL, sizeof(GUI_ARRAY)) == 0) {
			int i, NumColumns, NumBytes;
			GUI_ARRAY *pRow;
			const char *s;
			GUI_ARRAY_CREATE((GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, NumRows));  /* For higher debug levels only */
			/* Add columns for the new row */
			NumColumns = HEADER_GetNumItems(pObj->hHeader);
			for (i = 0; i < NumColumns; i++) {
				LISTVIEW_ITEM *pItem;
				pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, NumRows);
				s = (ppText) ? *ppText++ : 0;
				if (s == 0) {
					ppText = 0;
				}
				NumBytes = GUI__strlen(s) + 1;     /* 0 if no string is specified (s == NULL) */
				GUI_ARRAY_AddItem(pRow, NULL, sizeof(LISTVIEW_ITEM) + NumBytes);
				pItem = (LISTVIEW_ITEM *)GUI_ARRAY_GetpItem(pRow, i);
				if (NumBytes > 1) {
					GUI__strcpy(pItem->acText, s);
				}
			}
			LISTVIEW__UpdateScrollParas(pObj);
			LISTVIEW__InvalidateRow(pObj, NumRows);
		}

	}
}


LISTVIEW_Handle LISTVIEW_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int ExFlags) {
	return LISTVIEW_CreateEx(x0, y0, xsize, ysize, hParent, Flags, ExFlags, Id);
}
LISTVIEW_Handle LISTVIEW_CreateAttached(WM_HWIN hParent, int Id, int SpecialFlags) {
	return LISTVIEW_CreateEx(0, 0, 0, 0, hParent, WM_CF_SHOW, SpecialFlags, Id);
}

LISTVIEW_Handle LISTVIEW_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	LISTVIEW_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = LISTVIEW_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	return hThis;
}

const GUI_FONT  *LISTVIEW_SetDefaultFont(const GUI_FONT  *pFont) {
	const GUI_FONT  *pOldFont = LISTVIEW_DefaultProps.pFont;
	LISTVIEW_DefaultProps.pFont = pFont;
	return pOldFont;
}
RGB_COLOR LISTVIEW_SetDefaultTextColor(unsigned Index, RGB_COLOR Color) {
	RGB_COLOR OldColor = 0;
	if (Index < GUI_COUNTOF(LISTVIEW_DefaultProps.aTextColor)) {
		OldColor = LISTVIEW_DefaultProps.aTextColor[Index];
		LISTVIEW_DefaultProps.aTextColor[Index] = Color;
	}
	return OldColor;
}
RGB_COLOR LISTVIEW_SetDefaultBkColor(unsigned Index, RGB_COLOR Color) {
	RGB_COLOR OldColor = 0;
	if (Index < GUI_COUNTOF(LISTVIEW_DefaultProps.aBkColor)) {
		OldColor = LISTVIEW_DefaultProps.aBkColor[Index];
		LISTVIEW_DefaultProps.aBkColor[Index] = Color;
	}
	return OldColor;
}
RGB_COLOR LISTVIEW_SetDefaultGridColor(RGB_COLOR Color) {
	RGB_COLOR OldColor = LISTVIEW_DefaultProps.GridColor;
	LISTVIEW_DefaultProps.GridColor = Color;
	return OldColor;
}

void LISTVIEW_DeleteColumn(LISTVIEW_Handle hObj, unsigned Index) {
	if (hObj) {
		LISTVIEW_Obj *pObj;
		pObj = (hObj);
		if (Index < GUI_ARRAY_GetNumItems(&pObj->AlignArray)) {
			unsigned NumRows, i;
			GUI_ARRAY *pRow;
			HEADER_DeleteItem(pObj->hHeader, Index);
			GUI_ARRAY_DeleteItem(&pObj->AlignArray, Index);
			NumRows = GUI_ARRAY_GetNumItems(&pObj->RowArray);
			for (i = 0; i < NumRows; i++) {
				LISTVIEW_ITEM *pItem;
				pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, i);
				/* Delete attached info items */
				pItem = (LISTVIEW_ITEM *)GUI_ARRAY_GetpItem(pRow, Index);
				if (pItem->hItemInfo) {
					GUI_ALLOC_Free(pItem->hItemInfo);
				}
				/* Delete cell */
				GUI_ARRAY_DeleteItem(pRow, Index);
			}
			LISTVIEW__UpdateScrollParas(pObj);
			LISTVIEW__InvalidateInsideArea(pObj);
		}
	}
}


static void _InvalidateRowAndBelow(LISTVIEW_Obj *pObj, int Sel) {
	if (Sel >= 0) {
		GUI_RECT Rect;
		int HeaderHeight, RowDistY;
		HeaderHeight = HEADER_GetHeight(pObj->hHeader);
		RowDistY = LISTVIEW__GetRowDistY(pObj);
		WM_GetInsideRectExScrollbar(pObj, &Rect);
		Rect.y0 += HeaderHeight + (Sel - pObj->ScrollStateV.v) * RowDistY;
		WM_InvalidateRect(pObj, &Rect);
	}
}

void LISTVIEW_DeleteRow(LISTVIEW_Handle hObj, unsigned Index) {
	if (hObj) {
		LISTVIEW_Obj *pObj;
		unsigned NumRows;
		pObj = (hObj);
		NumRows = GUI_ARRAY_GetNumItems(&pObj->RowArray);
		if (Index < NumRows) {
			unsigned NumColumns, i;
			GUI_ARRAY *pRow;
			pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, Index);
			/* Delete attached info items */
			NumColumns = GUI_ARRAY_GetNumItems(pRow);
			for (i = 0; i < NumColumns; i++) {
				LISTVIEW_ITEM *pItem;
				pItem = (LISTVIEW_ITEM *)GUI_ARRAY_GetpItem(pRow, i);
				if (pItem->hItemInfo) {
					GUI_ALLOC_Free(pItem->hItemInfo);
				}
			}
			/* Delete row */
			GUI_ARRAY_Delete(pRow);
			GUI_ARRAY_DeleteItem(&pObj->RowArray, Index);
			/* Adjust properties */
			if (pObj->Sel == (signed int)Index) {
				pObj->Sel = -1;
			}
			if (pObj->Sel > (signed int)Index) {
				pObj->Sel--;
			}
			if (LISTVIEW__UpdateScrollParas(pObj)) {
				LISTVIEW__InvalidateInsideArea(pObj);
			}
			else {
				_InvalidateRowAndBelow(pObj, Index);
			}
		}
	}
}


RGB_COLOR LISTVIEW_GetBkColor(LISTVIEW_Handle hObj, unsigned Index) {
	RGB_COLOR Color = GUI_INVALID_COLOR;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		if (Index <= GUI_COUNTOF(pObj->Props.aBkColor)) {
			Color = pObj->Props.aBkColor[Index];
		}

	}
	return Color;
}

const GUI_FONT  *LISTVIEW_GetFont(LISTVIEW_Handle hObj) {
	const GUI_FONT  *pFont = NULL;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		pFont = pObj->Props.pFont;

	}
	return pFont;
}

HEADER_Handle LISTVIEW_GetHeader(LISTVIEW_Handle hObj) {
	HEADER_Handle hHeader = 0;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		hHeader = pObj->hHeader;

	}
	return hHeader;
}

unsigned LISTVIEW_GetNumColumns(LISTVIEW_Handle hObj) {
	unsigned NumColumns = 0;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		NumColumns = GUI_ARRAY_GetNumItems(&pObj->AlignArray);

	}
	return NumColumns;
}

unsigned LISTVIEW_GetNumRows(LISTVIEW_Handle hObj) {
	unsigned NumRows = 0;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		NumRows = GUI_ARRAY_GetNumItems(&pObj->RowArray);

	}
	return NumRows;
}

int LISTVIEW_GetSel(LISTVIEW_Handle hObj) {
	int r = -1;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		r = pObj->Sel;

	}
	return r;
}

RGB_COLOR LISTVIEW_GetTextColor(LISTVIEW_Handle hObj, unsigned Index) {
	RGB_COLOR Color = GUI_INVALID_COLOR;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		if (Index <= GUI_COUNTOF(pObj->Props.aTextColor)) {
			Color = pObj->Props.aTextColor[Index];
		}

	}
	return Color;
}

void LISTVIEW_SetBkColor(LISTVIEW_Handle hObj, unsigned int Index, RGB_COLOR Color) {
	if (hObj) {
		LISTVIEW_Obj *pObj;
		if (Index < GUI_COUNTOF(pObj->Props.aBkColor)) {

			pObj = (hObj);
			if (Color != pObj->Props.aBkColor[Index]) {
				pObj->Props.aBkColor[Index] = Color;
				LISTVIEW__InvalidateInsideArea(pObj);
			}

		}
	}
}

void LISTVIEW_SetColumnWidth(LISTVIEW_Handle hObj, unsigned int Index, int Width) {
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		HEADER_SetItemWidth(pObj->hHeader, Index, Width);

	}
}

void LISTVIEW_SetFont(LISTVIEW_Handle hObj, const GUI_FONT  *pFont) {
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		if (pFont != pObj->Props.pFont) {
			pObj->Props.pFont = pFont;
			LISTVIEW__UpdateScrollParas(pObj);
			LISTVIEW__InvalidateInsideArea(pObj);
		}

	}
}

int LISTVIEW_SetGridVis(LISTVIEW_Handle hObj, int Show) {
	int ShowGrid = 0;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		ShowGrid = pObj->ShowGrid;
		if (Show != ShowGrid) {
			pObj->ShowGrid = Show;
			LISTVIEW__UpdateScrollParas(pObj);
			LISTVIEW__InvalidateInsideArea(pObj);
		}

	}
	return ShowGrid;
}

static LISTVIEW_ITEM_INFO *_GetpItemInfo(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned int Index) {
	LISTVIEW_ITEM_INFO *pItemInfo = 0;
	LISTVIEW_ITEM *pItem;
	LISTVIEW_Obj *pObj;
	if (hObj) {
		if ((Column < LISTVIEW_GetNumColumns(hObj)) && (Row < LISTVIEW_GetNumRows(hObj)) && (Index < GUI_COUNTOF(pItemInfo->aTextColor))) {
			pObj = (hObj);
			pItem = (LISTVIEW_ITEM *)GUI_ARRAY_GetpItem((GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, Row), Column);
			if (!pItem->hItemInfo) {
				int i;
				pItem->hItemInfo = GUI_ALLOC_AllocZero(sizeof(LISTVIEW_ITEM_INFO));
				pItemInfo = (LISTVIEW_ITEM_INFO *)(pItem->hItemInfo);
				for (i = 0; i < GUI_COUNTOF(pItemInfo->aTextColor); i++) {
					pItemInfo->aTextColor[i] = LISTVIEW_GetTextColor(hObj, i);
					pItemInfo->aBkColor[i] = LISTVIEW_GetBkColor(hObj, i);
				}
			}
			else {
				pItemInfo = (LISTVIEW_ITEM_INFO *)(pItem->hItemInfo);
			}
		}
	}
	return pItemInfo;
}
void LISTVIEW_SetItemTextColor(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned int Index, RGB_COLOR Color) {
	LISTVIEW_ITEM_INFO *pItemInfo;

	pItemInfo = _GetpItemInfo(hObj, Column, Row, Index);
	if (pItemInfo) {
		pItemInfo->aTextColor[Index] = Color;
	}

}
void LISTVIEW_SetItemBkColor(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned int Index, RGB_COLOR Color) {
	LISTVIEW_ITEM_INFO *pItemInfo;

	pItemInfo = _GetpItemInfo(hObj, Column, Row, Index);
	if (pItemInfo) {
		pItemInfo->aBkColor[Index] = Color;
	}

}

void LISTVIEW_SetItemText(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, const char *s) {
	if (hObj) {
		if ((Column < LISTVIEW_GetNumColumns(hObj)) && (Row < LISTVIEW_GetNumRows(hObj))) {
			int NumBytes;
			LISTVIEW_ITEM *pItem;
			LISTVIEW_Obj *pObj;

			pObj = (hObj);
			NumBytes = GUI__strlen(s) + 1;
			pItem = (LISTVIEW_ITEM *)GUI_ARRAY_ResizeItem((GUI_ARRAY *)GUI_ARRAY_GetpItem(&pObj->RowArray, Row), Column, sizeof(LISTVIEW_ITEM) + NumBytes);
			if (NumBytes > 1) {
				GUI__strcpy(pItem->acText, s);
			}
			LISTVIEW__InvalidateRow(pObj, Row);

		}
	}
}

void LISTVIEW_SetLBorder(LISTVIEW_Handle hObj, unsigned BorderSize) {
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		if (pObj->LBorder != BorderSize) {
			pObj->LBorder = BorderSize;
			LISTVIEW__InvalidateInsideArea(pObj);
		}

	}
}

void LISTVIEW_SetRBorder(LISTVIEW_Handle hObj, unsigned BorderSize) {
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		if (pObj->RBorder != BorderSize) {
			pObj->RBorder = BorderSize;
			LISTVIEW__InvalidateInsideArea(pObj);
		}

	}
}

unsigned LISTVIEW_SetRowHeight(LISTVIEW_Handle hObj, unsigned RowHeight) {
	unsigned r = 0;
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		r = pObj->RowDistY;
		if (RowHeight != r) {
			pObj->RowDistY = RowHeight;
			LISTVIEW__UpdateScrollParas(pObj);
			LISTVIEW__InvalidateInsideArea(pObj);
		}

	}
	return r;
}

void LISTVIEW_SetSel(LISTVIEW_Handle hObj, int NewSel) {
	if (hObj) {
		LISTVIEW_Obj *pObj;
		int MaxSel;

		pObj = (hObj);
		MaxSel = GUI_ARRAY_GetNumItems(&pObj->RowArray) - 1;
		if (NewSel > MaxSel) {
			NewSel = MaxSel;
		}
		if (NewSel < 0) {
			NewSel = -1;
		}
		if (NewSel != pObj->Sel) {
			int OldSel;
			OldSel = pObj->Sel;
			pObj->Sel = NewSel;
			if (LISTVIEW__UpdateScrollPos(pObj)) {
				LISTVIEW__InvalidateInsideArea(pObj);
			}
			else {
				LISTVIEW__InvalidateRow(pObj, OldSel);
				LISTVIEW__InvalidateRow(pObj, NewSel);
			}
			WM_NotifyParent(hObj, WM_NOTIFICATION_SEL_CHANGED);
		}

	}
}

void LISTVIEW_SetTextAlign(LISTVIEW_Handle hObj, unsigned int Index, int Align) {
	if (hObj) {
		LISTVIEW_Obj *pObj;

		pObj = (hObj);
		if (Index < GUI_ARRAY_GetNumItems(&pObj->AlignArray)) {
			int *pAlign;
			pAlign = (int *)GUI_ARRAY_GetpItem(&pObj->AlignArray, Index);
			if (Align != *pAlign) {
				*pAlign = Align;
				LISTVIEW__InvalidateInsideArea(pObj);
			}
		}

	}
}

void LISTVIEW_SetTextColor(LISTVIEW_Handle hObj, unsigned int Index, RGB_COLOR Color) {
	if (hObj) {
		LISTVIEW_Obj *pObj;
		if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {

			pObj = (hObj);
			if (Color != pObj->Props.aTextColor[Index]) {
				pObj->Props.aTextColor[Index] = Color;
				LISTVIEW__InvalidateInsideArea(pObj);
			}

		}
	}
}

