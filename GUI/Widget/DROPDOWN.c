
#include <stdlib.h>
#include <string.h>

#include "GUIDebug.h"
#include "GUI_Protected.h"
#include "GUI_ARRAY.h"

#include "WM_Intern.h"

#include "WIDGET.h"
#include "LISTBOX.h"

#include "SCROLLBAR.h"

#include "DROPDOWN.h"
#include "DROPDOWN_Private.h"

/* Define default fonts */
#define DROPDOWN_FONT_DEFAULT &GUI_Font13_1
/* Define colors */
#define DROPDOWN_BKCOLOR0_DEFAULT GUI_WHITE     /* Not selected */
#define DROPDOWN_BKCOLOR1_DEFAULT GUI_GRAY      /* Selected, no focus */
#define DROPDOWN_BKCOLOR2_DEFAULT GUI_BLUE      /* Selected, focus */
#define DROPDOWN_TEXTCOLOR0_DEFAULT GUI_BLACK   /* Not selected */
#define DROPDOWN_TEXTCOLOR1_DEFAULT GUI_WHITE   /* Selected, no focus */
#define DROPDOWN_TEXTCOLOR2_DEFAULT GUI_WHITE   /* Selected, focus */
#define DROPDOWN_BORDER_DEFAULT 2
#define DROPDOWN_ALIGN_DEFAULT GUI_TA_LEFT      /* Default text alignment */
DROPDOWN_PROPS DROPDOWN__DefaultProps = {
  DROPDOWN_FONT_DEFAULT,
  DROPDOWN_BKCOLOR0_DEFAULT,
  DROPDOWN_BKCOLOR1_DEFAULT,
  DROPDOWN_BKCOLOR2_DEFAULT,
  DROPDOWN_TEXTCOLOR0_DEFAULT,
  DROPDOWN_TEXTCOLOR1_DEFAULT,
  DROPDOWN_TEXTCOLOR2_DEFAULT,
  DROPDOWN_BORDER_DEFAULT,
  DROPDOWN_ALIGN_DEFAULT
};

/*********************************************************************
*
*       _GetNumItems
  Returns:
	Number of fully or partially visible items
*/
static int _GetNumItems(DROPDOWN_Obj *pObj) {
	return pObj->Handles.NumItems;
}
/*********************************************************************
*
*       _GethItem
  Returns:
	Handle of the specified item
*/
static WM_HMEM _GethItem(DROPDOWN_Obj *pObj, int Index) {
	return GUI_ARRAY_GethItem(&pObj->Handles, Index);
}
static void _DrawTriangleDown(int x, int y, int Size) {
	for (; Size >= 0; Size--, y++) {
		GUI_DrawHLine(y, x - Size, x + Size);
	}
}
/*********************************************************************
*
*       _GetpItem
  Returns:
	Pointer to the specified item
*/
static const char *_GetpItem(DROPDOWN_Obj *pObj, int Index) {
	const char *s = NULL;
	WM_HMEM h = _GethItem(pObj, Index);
	if (h) {
		s = (const char *)(h);
	}
	return s;
}
static int _Tolower(int Key) {
	if ((Key >= 0x41) && (Key <= 0x5a)) {
		Key += 0x20;
	}
	return Key;
}
static void _SelectByKey(DROPDOWN_Handle hObj, int Key) {
	int i;
	DROPDOWN_Obj *pObj;
	pObj = (hObj);
	Key = _Tolower(Key);
	for (i = 0; i < _GetNumItems(pObj); i++) {
		char c = _Tolower(*_GetpItem(pObj, i));
		if (c == Key) {
			DROPDOWN_SetSel(hObj, i);
			break;
		}
	}
}
static void _FreeAttached(DROPDOWN_Obj *pObj) {
	GUI_ARRAY_Delete(&pObj->Handles);
	WM_DeleteWindow(pObj->hListWin);
}
static void _Paint(DROPDOWN_Handle hObj) {
	int Border;
	GUI_RECT r;
	const char *s;
	int InnerSize, ColorIndex;
	DROPDOWN_Obj *pObj;
	int TextBorderSize;
	/* Do some initial calculations */
	pObj = (hObj);
	Border = pObj->Widget.pEffect->EffectSize;
	TextBorderSize = pObj->Props.TextBorderSize;
	GUI_SetFont(pObj->Props.pFont);
	ColorIndex = (pObj->Widget.State & WIDGET_STATE_FOCUS) ? 2 : 1;
	s = _GetpItem(pObj, pObj->Sel);
	WM_GetClientRect(&r);
	GUI__ReduceRect(&r, &r, Border);
	InnerSize = r.y1 - r.y0 + 1;
	/* Draw the 3D effect (if configured) */
	WIDGET__EFFECT_DrawDown(&pObj->Widget);
	/* Draw the outer text frames */
	r.x1 -= InnerSize;     /* Spare square area to the right */
	LCD_SetColor(pObj->Props.aBackColor[ColorIndex]);
	/* Draw the text */
	LCD_SetBkColor(pObj->Props.aBackColor[ColorIndex]);
	GUI_FillRectEx(&r);
	r.x0 += TextBorderSize;
	r.x1 -= TextBorderSize;
	LCD_SetColor(pObj->Props.aTextColor[ColorIndex]);
	GUI_DispStringInRect(s, &r, pObj->Props.Align);/**/
	/* Draw arrow */
	WM_GetClientRect(&r);
	GUI__ReduceRect(&r, &r, Border);
	r.x0 = r.x1 + 1 - InnerSize;
	LCD_SetColor(0xc0c0c0);
	GUI_FillRectEx(&r);
	LCD_SetColor(GUI_BLACK);
	_DrawTriangleDown((r.x1 + r.x0) / 2, r.y0 + 5, (r.y1 - r.y0 - 8) / 2);
	WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &r);
}
static int _OnTouch(DROPDOWN_Handle hObj, WM_MESSAGE *pMsg) {
	const GUI_PID_STATE *pState = (const GUI_PID_STATE *)pMsg->Data.p;
	if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
		}
		else {
			WM_NotifyParent(hObj, WM_NOTIFICATION_RELEASED);
		}
	}
	else {     /* Mouse moved out */
		WM_NotifyParent(hObj, WM_NOTIFICATION_MOVED_OUT);
	}
	return 0; /* Message handled */
}
void DROPDOWN__AdjustHeight(DROPDOWN_Handle hObj, DROPDOWN_Obj *pObj) {
	int Height;
	Height = pObj->TextHeight;
	if (!Height) {
		Height = GUI_GetYDistOfFont(pObj->Props.pFont);
	}
	Height += pObj->Widget.pEffect->EffectSize + 2 * pObj->Props.TextBorderSize;
	WM_SetSize(hObj, WM__GetWindowSizeX(&pObj->Widget.Win), Height);
}
static void _DROPDOWN_Callback(WM_MESSAGE *pMsg) {
	DROPDOWN_Handle hObj = pMsg->hWin;
	DROPDOWN_Obj *pObj = (hObj);
	char IsExpandedBeforeMsg;
	IsExpandedBeforeMsg = pObj->hListWin ? 1 : 0;
	/* Let widget handle the standard messages */
	if (WIDGET_HandleActive(hObj, pMsg) == 0) {
		return;
	}
	switch (pMsg->MsgId) {
		case WM_NOTIFY_PARENT:
			switch (pMsg->Data.v) {
				case WM_NOTIFICATION_SCROLL_CHANGED:
					WM_NotifyParent(hObj, WM_NOTIFICATION_SCROLL_CHANGED);
					break;
				case WM_NOTIFICATION_CLICKED:
					DROPDOWN_SetSel(hObj, LISTBOX_GetSel(pObj->hListWin));
					WM_SetFocus(hObj);
					break;
				case LISTBOX_NOTIFICATION_LOST_FOCUS:
					DROPDOWN_Collapse(hObj);
					break;
			}
			break;
		case WM_PID_STATE_CHANGED:
			if (IsExpandedBeforeMsg == 0) {    /* Make sure we do not react a second time */
				const WM_PID_STATE_CHANGED_INFO *pInfo = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
				if (pInfo->State) {
					DROPDOWN_Expand(hObj);
				}
			}
			break;
		case WM_TOUCH:
			if (_OnTouch(hObj, pMsg) == 0) {
				return;
			}
			break;
		case WM_PAINT:
			_Paint(hObj);
			break;
		case WM_DELETE:
			_FreeAttached(pObj);
			break;       /* No return here ... WM_DefaultProc needs to be called */
		case WM_KEY:
			if (((const WM_KEY_INFO *)(pMsg->Data.p))->PressedCnt > 0) {
				int Key = ((const WM_KEY_INFO *)(pMsg->Data.p))->Key;
				switch (Key) {
					case GUI_KEY_TAB:
						break;                    /* Send to parent by not doing anything */
					default:
						DROPDOWN_AddKey(hObj, Key);
						return;
				}
			}
			break;
	}
	WM_DefaultProc(pMsg);
}
DROPDOWN_Handle DROPDOWN_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
								  int WinFlags, int ExFlags, int Id) {
	DROPDOWN_Handle hObj;
	hObj = WM_CreateWindowAsChild(x0, y0, xsize, -1, hParent, WinFlags, _DROPDOWN_Callback,
								  sizeof(DROPDOWN_Obj) - sizeof(WM_Obj));
	if (hObj) {
		DROPDOWN_Obj *pObj;

		pObj = (hObj);
		/* Init sub-classes */
		GUI_ARRAY_CREATE(&pObj->Handles);
		/* init widget specific variables */
		WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
		pObj->Flags = ExFlags;
		pObj->Props = DROPDOWN__DefaultProps;
		pObj->ScrollbarWidth = 0;
		pObj->ySizeEx = ysize;
		DROPDOWN__AdjustHeight(hObj, pObj);

	}
	return hObj;
}
void DROPDOWN_Collapse(DROPDOWN_Handle hObj) {
	DROPDOWN_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		if (pObj->hListWin) {
			WM_DeleteWindow(pObj->hListWin);
			pObj->hListWin = 0;
		}

	}
}
void DROPDOWN_Expand(DROPDOWN_Handle hObj) {
	int xSize, ySize, i, NumItems;
	WM_HWIN hLst;
	GUI_RECT r;
	WM_HWIN hParent;
	WM_Obj *pParent;
	DROPDOWN_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		if (pObj->hListWin == 0) {
			hParent = WM_GetParent(hObj);
			pParent = (hParent);
			xSize = WM__GetWindowSizeX(&pObj->Widget.Win);
			ySize = pObj->ySizeEx;
			NumItems = _GetNumItems(pObj);
			/* Get coordinates of window in client coordiantes of parent */
			r = pObj->Widget.Win.Rect;
			GUI_MoveRect(&r, -pParent->Rect.x0, -pParent->Rect.y0);
			if (pObj->Flags & DROPDOWN_CF_UP) {
				r.y0 -= ySize;
			}
			else {
				r.y0 = r.y1;
			}
			hLst = LISTBOX_CreateAsChild(NULL, WM_GetParent(hObj), r.x0, r.y0
										 , xSize, ySize, WM_CF_SHOW);
			if (pObj->Flags & DROPDOWN_SF_AUTOSCROLLBAR) {
				LISTBOX_SetScrollbarWidth(hLst, pObj->ScrollbarWidth);
				LISTBOX_SetAutoScrollV(hLst, 1);
			}
			for (i = 0; i < NumItems; i++) {
				LISTBOX_AddString(hLst, _GetpItem(pObj, i));
			}
			for (i = 0; i < GUI_COUNTOF(pObj->Props.aBackColor); i++) {
				LISTBOX_SetBkColor(hLst, i, pObj->Props.aBackColor[i]);
			}
			for (i = 0; i < GUI_COUNTOF(pObj->Props.aTextColor); i++) {
				LISTBOX_SetTextColor(hLst, i, pObj->Props.aTextColor[i]);
			}
			LISTBOX_SetItemSpacing(hLst, pObj->ItemSpacing);
			LISTBOX_SetFont(hLst, pObj->Props.pFont);
			WM_SetFocus(hLst);
			pObj->hListWin = hLst;
			LISTBOX_SetOwner(hLst, hObj);
			LISTBOX_SetSel(hLst, pObj->Sel);
			WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
		}

	}
}
void DROPDOWN_AddKey(DROPDOWN_Handle hObj, int Key) {
	if (hObj) {

		switch (Key) {
			case GUI_KEY_DOWN:
				DROPDOWN_IncSel(hObj);
				break;
			case GUI_KEY_UP:
				DROPDOWN_DecSel(hObj);
				break;
			default:
				_SelectByKey(hObj, Key);
				break;
		}

	}
}
void DROPDOWN_AddString(DROPDOWN_Handle hObj, const char *s) {
	DROPDOWN_Obj *pObj;
	if (hObj && s) {

		pObj = (hObj);
		GUI_ARRAY_AddItem(&pObj->Handles, s, strlen(s) + 1);
		DROPDOWN_Invalidate(hObj);

	}
}
int DROPDOWN_GetNumItems(DROPDOWN_Handle hObj) {
	DROPDOWN_Obj *pObj;
	int r = 0;
	if (hObj) {

		pObj = (hObj);
		r = _GetNumItems(pObj);

	}
	return r;
}
void DROPDOWN_SetFont(DROPDOWN_Handle hObj, const GUI_FONT GUI_UNI_PTR *pfont) {
	int OldHeight;
	DROPDOWN_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		OldHeight = GUI_GetYDistOfFont(pObj->Props.pFont);
		pObj->Props.pFont = pfont;
		DROPDOWN__AdjustHeight(hObj, pObj);
		DROPDOWN_Invalidate(hObj);
		if (pObj->hListWin) {
			if (OldHeight != GUI_GetYDistOfFont(pObj->Props.pFont)) {
				DROPDOWN_Collapse(hObj);
				DROPDOWN_Expand(hObj);
			}
			LISTBOX_SetFont(pObj->hListWin, pfont);
		}

	}
}
void DROPDOWN_SetBkColor(DROPDOWN_Handle hObj, unsigned int Index, RGB_COLOR color) {
	DROPDOWN_Obj *pObj;
	if (hObj) {
		if (Index < GUI_COUNTOF(pObj->Props.aBackColor)) {

			pObj = (hObj);
			pObj->Props.aBackColor[Index] = color;
			DROPDOWN_Invalidate(hObj);
			if (pObj->hListWin) {
				LISTBOX_SetBkColor(pObj->hListWin, Index, color);
			}

		}
	}
}
void DROPDOWN_SetTextColor(DROPDOWN_Handle hObj, unsigned int Index, RGB_COLOR color) {
	DROPDOWN_Obj *pObj;
	if (hObj) {
		if (Index < GUI_COUNTOF(pObj->Props.aBackColor)) {

			pObj = (hObj);
			pObj->Props.aTextColor[Index] = color;
			DROPDOWN_Invalidate(hObj);
			if (pObj->hListWin) {
				LISTBOX_SetTextColor(pObj->hListWin, Index, color);
			}

		}
	}
}
void DROPDOWN_SetSel(DROPDOWN_Handle hObj, int Sel) {
	int NumItems, MaxSel;
	DROPDOWN_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		NumItems = _GetNumItems(pObj);
		MaxSel = NumItems ? NumItems - 1 : 0;
		if (Sel > MaxSel) {
			Sel = MaxSel;
		}
		if (Sel != pObj->Sel) {
			pObj->Sel = Sel;
			DROPDOWN_Invalidate(hObj);
			WM_NotifyParent(hObj, WM_NOTIFICATION_SEL_CHANGED);
		}

	}
}
void DROPDOWN_IncSel(DROPDOWN_Handle hObj) {
	int Sel = DROPDOWN_GetSel(hObj);
	DROPDOWN_SetSel(hObj, Sel + 1);
}
void DROPDOWN_DecSel(DROPDOWN_Handle hObj) {
	int Sel = DROPDOWN_GetSel(hObj);
	if (Sel)
		Sel--;
	DROPDOWN_SetSel(hObj, Sel);
}
int  DROPDOWN_GetSel(DROPDOWN_Handle hObj) {
	int r = 0;
	DROPDOWN_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		r = pObj->Sel;

	}
	return r;
}
void DROPDOWN_SetScrollbarWidth(DROPDOWN_Handle hObj, unsigned Width) {
	DROPDOWN_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		if (Width != (unsigned)pObj->ScrollbarWidth) {
			pObj->ScrollbarWidth = Width;
			if (pObj->hListWin) {
				LISTBOX_SetScrollbarWidth(pObj->hListWin, Width);
			}
		}

	}
}
void DROPDOWN_SetDefaultFont(const GUI_FONT *pFont) {
	DROPDOWN__DefaultProps.pFont = pFont;
}
const GUI_FONT GUI_UNI_PTR *DROPDOWN_GetDefaultFont(void) {
	return DROPDOWN__DefaultProps.pFont;
}

DROPDOWN_Handle DROPDOWN_Create(WM_HWIN hWinParent, int x0, int y0, int xsize, int ysize, int Flags) {
	return DROPDOWN_CreateEx(x0, y0, xsize, ysize, hWinParent, Flags, 0, 0);
}

DROPDOWN_Handle DROPDOWN_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo,
										WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	DROPDOWN_Handle  hThis;
	GUI_USE_PARA(cb);
	hThis = DROPDOWN_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0,
							  pCreateInfo->xSize, pCreateInfo->ySize,
							  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	return hThis;
}

void DROPDOWN_DeleteItem(DROPDOWN_Handle hObj, unsigned int Index) {
	if (hObj) {
		DROPDOWN_Obj *pObj;
		unsigned int NumItems;
		NumItems = DROPDOWN_GetNumItems(hObj);
		if (Index < NumItems) {

			pObj = (hObj);
			GUI_ARRAY_DeleteItem(&pObj->Handles, Index);
			WM_InvalidateWindow(hObj);
			if (pObj->hListWin) {
				LISTBOX_DeleteItem(pObj->hListWin, Index);
			}

		}
	}
}

void DROPDOWN_InsertString(DROPDOWN_Handle hObj, const char *s, unsigned int Index) {
	if (hObj && s) {
		DROPDOWN_Obj *pObj;
		unsigned int NumItems;

		pObj = (hObj);
		NumItems = DROPDOWN_GetNumItems(hObj);
		if (Index < NumItems) {
			WM_HMEM hItem;
			hItem = GUI_ARRAY_InsertItem(&pObj->Handles, Index, strlen(s) + 1);
			if (hItem) {
				char *pBuffer = (char *)(hItem);
				strcpy(pBuffer, s);
			}
			WM_InvalidateWindow(hObj);
			if (pObj->hListWin) {
				LISTBOX_InsertString(pObj->hListWin, s, Index);
			}
		}
		else {
			DROPDOWN_AddString(hObj, s);
			if (pObj->hListWin) {
				LISTBOX_AddString(pObj->hListWin, s);
			}
		}

	}
}

void DROPDOWN_SetItemSpacing(DROPDOWN_Handle hObj, unsigned Value) {
	if (hObj) {
		DROPDOWN_Obj *pObj;

		pObj = (hObj);
		pObj->ItemSpacing = Value;
		if (pObj->hListWin) {
			LISTBOX_SetItemSpacing(pObj->hListWin, Value);
		}

	}
}
unsigned DROPDOWN_GetItemSpacing(DROPDOWN_Handle hObj) {
	unsigned Value = 0;
	if (hObj) {
		DROPDOWN_Obj *pObj;

		pObj = (hObj);
		Value = pObj->ItemSpacing;

	}
	return Value;
}

void DROPDOWN_SetAutoScroll(DROPDOWN_Handle hObj, int OnOff) {
	if (hObj) {
		DROPDOWN_Obj *pObj;
		char Flags;

		pObj = (hObj);
		Flags = pObj->Flags & (~DROPDOWN_SF_AUTOSCROLLBAR);
		if (OnOff) {
			Flags |= DROPDOWN_SF_AUTOSCROLLBAR;
		}
		if (pObj->Flags != Flags) {
			pObj->Flags = Flags;
			if (pObj->hListWin) {
				LISTBOX_SetAutoScrollV(pObj->hListWin, (Flags & DROPDOWN_SF_AUTOSCROLLBAR) ? 1 : 0);
			}
		}

	}
}

void DROPDOWN_SetTextAlign(DROPDOWN_Handle hObj, int Align) {
	DROPDOWN_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		pObj->Props.Align = Align;
		WM_Invalidate(hObj);

	}
}

void DROPDOWN_SetTextHeight(DROPDOWN_Handle hObj, unsigned TextHeight) {
	DROPDOWN_Obj *pObj;
	if (hObj) {

		pObj = (hObj);
		pObj->TextHeight = TextHeight;
		DROPDOWN__AdjustHeight(hObj, pObj);
		WM_Invalidate(hObj);

	}
}

