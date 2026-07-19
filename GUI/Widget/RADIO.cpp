#include "GUI.h"
#include "GUI_Protected.h"

#include "RADIO.h"
#include "RADIO_Private.h"

/* Define default image inactiv */
#define RADIO_IMAGE0_DEFAULT        &RADIO__abmRadio[0]
/* Define default image activ */
#define RADIO_IMAGE1_DEFAULT        &RADIO__abmRadio[1]
/* Define default image check */
#define RADIO_IMAGE_CHECK_DEFAULT   &RADIO__bmCheck
/* Define default font */
#define RADIO_FONT_DEFAULT          &GUI_Font13_1
/* Define default text color */
#define RADIO_DEFAULT_TEXT_COLOR    RGB_BLACK
/* Define default background color */
#define RADIO_DEFAULT_BKCOLOR       RGB_GRAYL(0xC0)
#define RADIO_BORDER                  2
tRADIO_SetValue *RADIO__pfHandleSetValue;
RGB_COLOR         RADIO__DefaultTextColor = RADIO_DEFAULT_TEXT_COLOR;
const GUI_FONT *RADIO__pDefaultFont = RADIO_FONT_DEFAULT;
const GUI_BITMAP *RADIO__apDefaultImage[] = { RADIO_IMAGE0_DEFAULT, RADIO_IMAGE1_DEFAULT };
const GUI_BITMAP *RADIO__pDefaultImageCheck = RADIO_IMAGE_CHECK_DEFAULT;

static void _ResizeRect(GUI_RECT *pDest, const GUI_RECT *pSrc, int Diff) {
	pDest->y0 = pSrc->y0 - Diff;
	pDest->y1 = pSrc->y1 + Diff;
	pDest->x0 = pSrc->x0 - Diff;
	pDest->x1 = pSrc->x1 + Diff;
}
/*********************************************************************
*
*       _OnPaint
*
* Purpose:
*   Paints the RADIO button.
*   The button can actually consist of multiple buttons (NumItems).
*   The focus rectangle will be drawn on top of the text if any text is set,
*   otherwise around the entire buttons.
*/
static void _OnPaint(RADIO_Obj *pObj) {
	const GUI_BITMAP *pBmRadio;
	const GUI_BITMAP *pBmCheck;
	const char *pText;
	GUI_FONTINFO FontInfo;
	GUI_RECT Rect, r, rFocus;
	int i, y, HasFocus, FontDistY;
	uint16_t SpaceAbove, CHeight, FocusBorder;
	/* Init some data */
	rFocus = WIDGET__GetClientRect(pObj);
	HasFocus = (pObj->State & WIDGET_STATE_FOCUS) ? 1 : 0;
	pBmRadio = pObj->apBmRadio[WM_IsEnabled(pObj)];
	pBmCheck = pObj->pBmCheck;
	rFocus.x1 = pBmRadio->XSize + RADIO_BORDER * 2 - 1;
	rFocus.y1 = pObj->Height + ((pObj->NumItems - 1) * pObj->Spacing) - 1;
	/* Select font and text color */
	GUI_SetColor(pObj->TextColor);
	GUI_SetFont(pObj->pFont);
	GUI_SetTextMode(DRAWMODE_TRANS);
	/* Get font infos */
	GUI_GetFontInfo(pObj->pFont, &FontInfo);
	FontDistY = GUI_GetFontDistY();
	CHeight = FontInfo.CHeight;
	SpaceAbove = FontInfo.Baseline - CHeight;
	Rect.x0 = pBmRadio->XSize + RADIO_BORDER * 2 + 2;
	Rect.y0 = (CHeight <= pObj->Height) ? ((pObj->Height - CHeight) / 2) : 0;
	Rect.y1 = Rect.y0 + CHeight - 1;
	FocusBorder = (FontDistY <= 12) ? 2 : 3;
	if (Rect.y0 < FocusBorder) {
		FocusBorder = Rect.y0;
	}
	/* Clear inside ... Just in case      */
	/* Fill with parents background color */
#if WM_SUPPORT_TRANSPARENCY
	if (!WM_GetHasTrans(pObj))
#endif
	{
		if (pObj->BkColor != GUI_INVALID_COLOR) {
			GUI_SetBkColor(pObj->BkColor);
		}
		else {
			GUI_SetBkColor(RADIO_DEFAULT_BKCOLOR);
		}
		GUI_Clear();
	}
	/* Iterate over all items */
	for (i = 0; i < pObj->NumItems; i++) {
		y = i * pObj->Spacing;
		/* Draw the radio button bitmap */
		GUI_DrawBitmap(pBmRadio, RADIO_BORDER, RADIO_BORDER + y);
		/* Draw the check bitmap */
		if (pObj->Sel == i) {
			GUI_DrawBitmap(pBmCheck, RADIO_BORDER + (pBmRadio->XSize - pBmCheck->XSize) / 2,
						   RADIO_BORDER + ((pBmRadio->YSize - pBmCheck->YSize) / 2) + y);
		}
		/* Draw text if available */
		pText = (const char *)GUI_ARRAY_GetpItem(&pObj->TextArray, i);
		if (pText) {
			if (*pText) {
				r = Rect;
				r.x1 = r.x0 + GUI_GetStringDistX(pText) - 2;
				r += GUI_POINT{0, y};
				GUI_DispStringAt(pText, r.x0, r.y0 - SpaceAbove);
				/* Calculate focus rect */
				if (HasFocus && (pObj->Sel == i)) {
					_ResizeRect(&rFocus, &r, FocusBorder);
				}
			}
		}
	}
	/* Draw the focus rect */
	if (HasFocus) {
		GUI_SetColor(RGB_BLACK);
		WIDGET__DrawFocusRect(pObj, rFocus, 0);
	}
}
static void _OnTouch(RADIO_Obj *pObj, const GUI_PID_STATE *pState) {
	int Notification;
	int Hit = 0;
	if (pState) {  /* Something happened in our area (pressed or released) */
		if (pState->Pressed) {
			int y, Sel;
			y = pState->y;
			Sel = y / pObj->Spacing;
			y -= Sel * pObj->Spacing;
			if (y <= pObj->Height) {
				RADIO_SetValue(pObj, Sel);
			}
			if (WM_IsFocussable(pObj)) {
				WM_SetFocus(pObj);
			}
			Notification = WM_NOTIFICATION_CLICKED;
		}
		else {
			Hit = 1;
			Notification = WM_NOTIFICATION_RELEASED;
		}
	}
	else {
		Notification = WM_NOTIFICATION_MOVED_OUT;
	}
	WM_NotifyParent(pObj, Notification);
	if (Hit == 1) {
		GUI_StoreKey(pObj->Id);
	}
}
static char _OnKey(RADIO_Obj *pObj, const WM_KEY_INFO *pInfo) {
	if (pInfo->PressedCnt > 0) {
		switch (pInfo->Key) {
			case GUI_KEY_RIGHT:
			case GUI_KEY_DOWN:
				RADIO_Inc(pObj);
				return 1;
			case GUI_KEY_LEFT:
			case GUI_KEY_UP:
				RADIO_Dec(pObj);
				return 1;
		}
	}
	return 0;
}
static WM_PARAM _RADIO_Callback(WM_HWIN hWin, int MsgId, WM_PARAM Data) {
	RADIO_Obj *pObj = (RADIO_Obj *)hWin;
	/* Let widget handle the standard messages */
	if (!WIDGET_HandleActive(pObj, MsgId, &Data))
		return Data;
	switch (MsgId) {
		case WM_PAINT:
			_OnPaint(pObj);
			return 0;
		case WM_GET_RADIOGROUP:
			return pObj->GroupId;
		case WM_TOUCH:
			_OnTouch(pObj, (const GUI_PID_STATE *)Data);
			return 0;
		case WM_KEY:
			if (_OnKey(pObj, (const WM_KEY_INFO *)Data))
				return 0;
			break;
		case WM_DELETE:
			GUI_ARRAY_Delete(&pObj->TextArray);
			return 0;
	}
	return WM_DefaultProc(hWin, MsgId, Data);
}
void RADIO__SetValue(RADIO_Obj *pObj, int v) {
	if (v >= pObj->NumItems) {
		v = (int)pObj->NumItems - 1;
	}
	if (v != pObj->Sel) {
		pObj->Sel = v;
		WM_Invalidate(pObj);
		WM_NotifyParent(pObj, WM_NOTIFICATION_VALUE_CHANGED);
	}
}
/* Note: the parameters to a create function may vary.
		 Some widgets may have multiple create functions */
RADIO_Handle RADIO_CreateEx(int x0, int y0, int xSize, int ySize, WM_HWIN hParent,
							int WinFlags, int ExFlags, int Id, int NumItems, int Spacing) {
	RADIO_Handle hObj;
	int Height, i;
	/* Calculate helper variables */
	Height = RADIO__apDefaultImage[0]->YSize + RADIO_BORDER * 2;
	Spacing = (Spacing <= 0) ? 20 : Spacing;
	NumItems = (NumItems <= 0) ? 2 : NumItems;
	if (ySize == 0) {
		ySize = Height + ((NumItems - 1) * Spacing);
	}
	if (xSize == 0) {
		xSize = RADIO__apDefaultImage[0]->XSize + RADIO_BORDER * 2;
	}
#if WM_SUPPORT_TRANSPARENCY
	WinFlags |= WM_CF_HASTRANS;
#endif
	/* Create the window */
	hObj = WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent, WinFlags, _RADIO_Callback, sizeof(RADIO_Obj) - sizeof(WM_Obj));
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		/* Init sub-classes */
		GUI_ARRAY_CREATE(&pObj->TextArray);
		for (i = 0; i < NumItems; i++) {
			GUI_ARRAY_AddItem(&pObj->TextArray, NULL, 0);
		}
		/* Init widget specific variables */
		ExFlags &= RADIO_TEXTPOS_LEFT;
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE | ExFlags);
		/* Init member variables */
		pObj->apBmRadio[0] = RADIO__apDefaultImage[0];
		pObj->apBmRadio[1] = RADIO__apDefaultImage[1];
		pObj->pBmCheck = RADIO__pDefaultImageCheck;
		pObj->pFont = RADIO__pDefaultFont;
		pObj->TextColor = RADIO__DefaultTextColor;
		pObj->BkColor = WM_GetBkColor(hParent);
		pObj->NumItems = NumItems;
		pObj->Spacing = Spacing;
		pObj->Height = Height;

	}
	else {
	}
	return hObj;
}
void RADIO_AddValue(RADIO_Handle hObj, int Add) {
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		RADIO_SetValue(hObj, pObj->Sel + Add);

	}
}
void RADIO_Dec(RADIO_Handle hObj) {
	RADIO_AddValue(hObj, -1);
}
void RADIO_Inc(RADIO_Handle hObj) {
	RADIO_AddValue(hObj, 1);
}
void RADIO_SetValue(RADIO_Handle hObj, int v) {
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		if (pObj->GroupId && RADIO__pfHandleSetValue) {
			(*RADIO__pfHandleSetValue)(pObj, v);
		}
		else {
			if (v < 0) {
				v = 0;
			}
			RADIO__SetValue(pObj, v);
		}

	}
}
int RADIO_GetValue(RADIO_Handle hObj) {
	int r = 0;
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		r = pObj->Sel;

	}
	return r;
}

RADIO_Handle RADIO_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, unsigned Para) {
	return RADIO_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id, Para & 0xFF, (Para >> 8) & 0xFF);
}

RADIO_Handle RADIO_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	RADIO_Handle  hThis;
	int NumItems = (pCreateInfo->Para) & 0xFF;
	int Spacing = (pCreateInfo->Para >> 8) & 0xFF;
	GUI_USE_PARA(cb);
	hThis = RADIO_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
						   hWinParent, pCreateInfo->Flags, 0, pCreateInfo->Id, NumItems, Spacing);
	return hThis;
}

const GUI_FONT *RADIO_GetDefaultFont(void) {
	return RADIO__pDefaultFont;
}
RGB_COLOR RADIO_GetDefaultTextColor(void) {
	return RADIO__DefaultTextColor;
}
void RADIO_SetDefaultFont(const GUI_FONT *pFont) {
	RADIO__pDefaultFont = pFont;
}
void RADIO_SetDefaultTextColor(RGB_COLOR TextColor) {
	RADIO__DefaultTextColor = TextColor;
}


void RADIO_SetBkColor(RADIO_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		if (Color != pObj->BkColor) {
			pObj->BkColor = Color;
#if WM_SUPPORT_TRANSPARENCY
			if (Color <= RGB_WHITE) {
				WM_SetTransState(hObj, 0);
			}
			else {
				WM_SetTransState(hObj, WM_CF_HASTRANS);
			}
#endif
			WM_Invalidate(hObj);
		}

	}
}

void RADIO_SetDefaultImage(const GUI_BITMAP *pBitmap, unsigned int Index) {
	switch (Index) {
		case RADIO_BI_INACTIV:
		case RADIO_BI_ACTIV:
			RADIO__apDefaultImage[Index] = pBitmap;
			break;
		case RADIO_BI_CHECK:
			RADIO__pDefaultImageCheck = pBitmap;
			break;
	}
}

void RADIO_SetFont(RADIO_Handle hObj, const GUI_FONT *pFont) {
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		if (pFont != pObj->pFont) {
			pObj->pFont = pFont;
			if (GUI_ARRAY_GetNumItems(&pObj->TextArray))
				WM_Invalidate(hObj);
		}
	}
}

static void _SetValue(RADIO_Handle hObj, int v) {
	RADIO_Obj *pObj = (RADIO_Obj *)hObj;
	RADIO__SetValue(pObj, v);
}
static int _IsInGroup(WM_HWIN hWin, uint8_t GroupId) {
	if (GroupId)
		return WM_SendMessage(hWin, WM_GET_RADIOGROUP, 0) == GroupId;
	return 0;
}
static WM_HWIN _GetPrevInGroup(WM_HWIN hWin, uint8_t GroupId) {
	for (hWin = WM_GetPrevSibling(hWin); hWin; hWin = WM_GetPrevSibling(hWin)) {
		if (_IsInGroup(hWin, GroupId)) {
			return hWin;
		}
	}
	return 0;
}
static WM_HWIN _GetNextInGroup(WM_HWIN hWin, uint8_t GroupId) {
	for (; hWin; hWin = WM_GetNextSibling(hWin))
		if (_IsInGroup(hWin, GroupId))
			return hWin;
	return 0;
}
static void _ClearSelection(RADIO_Handle hObj, uint8_t GroupId) {
	for (auto pWin = (WM_Obj *)WM__GetFirstSibling(hObj); pWin; pWin = (WM_Obj *)pWin->hNext) {
		if (pWin != (WM_Obj *)hObj)
			if (_IsInGroup((WM_HWIN)pWin, GroupId))
				RADIO__SetValue((RADIO_Obj *)pWin, -1);
	}
}
static void _HandleSetValue(RADIO_Obj *pObj, int v) {
	if (v < 0) {
		WM_HWIN hWin = _GetPrevInGroup(pObj, pObj->GroupId);
		if (hWin) {
			WM_SetFocus(hWin);
			_SetValue(hWin, 0x7FFF);
			RADIO__SetValue(pObj, -1);
		}
	}
	else if (v >= pObj->NumItems) {
		WM_HWIN hWin = _GetNextInGroup(pObj->hNext, pObj->GroupId);
		if (hWin) {
			WM_SetFocus(hWin);
			_SetValue(hWin, 0);
			RADIO__SetValue(pObj, -1);
		}
	}
	else {
		if (pObj->Sel != v) {
			_ClearSelection(pObj, pObj->GroupId);
			RADIO__SetValue(pObj, v);
		}
	}
}

void RADIO_SetGroupId(RADIO_Handle hObj, uint8_t NewGroupId) {
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		uint8_t OldGroupId;
		OldGroupId = pObj->GroupId;
		if (NewGroupId != OldGroupId) {
			WM_HWIN hFirst;
			hFirst = WM__GetFirstSibling(hObj);
			/* Set function pointer if necessary */
			if (NewGroupId && (RADIO__pfHandleSetValue == NULL)) {
				RADIO__pfHandleSetValue = _HandleSetValue;
			}
			/* Pass our selection, if we have one, to another radio button in */
			/* our old group. So the group have a valid selection when we leave it. */
			if (OldGroupId && (pObj->Sel >= 0)) {
				WM_HWIN hWin;
				pObj->GroupId = 0; /* Leave group first, so _GetNextInGroup() could */
				/* not find a handle to our own window. */
				hWin = _GetNextInGroup(hFirst, OldGroupId);
				if (hWin) {
					_SetValue(hWin, 0);
				}
			}
			/* Make sure we have a valid selection according to our new group */
			if (_GetNextInGroup(hFirst, NewGroupId) != 0) {
				/* Join an existing group with an already valid selection, so clear our own one */
				RADIO__SetValue(pObj, -1);
			}
			else if (pObj->Sel < 0) {
				/* We are the first window in group, so we must have a valid selection at our own. */
				RADIO__SetValue(pObj, 0);
			}
			/* Change the group */
			pObj->GroupId = NewGroupId;
		}
	}
}


void RADIO_SetImage(RADIO_Handle hObj, const GUI_BITMAP *pBitmap, unsigned int Index) {
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		switch (Index) {
			case RADIO_BI_INACTIV:
			case RADIO_BI_ACTIV:
				pObj->apBmRadio[Index] = pBitmap;
				break;
			case RADIO_BI_CHECK:
				pObj->pBmCheck = pBitmap;
				break;
		}
		WM_Invalidate(hObj);

	}
}

void RADIO_SetText(RADIO_Handle hObj, const char *pText, unsigned Index) {
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		if (Index < (unsigned)pObj->NumItems) {
			GUI_ARRAY_SetItem(&pObj->TextArray, Index, pText, pText ? (GUI__strlen(pText) + 1) : 0);
			WM_Invalidate(hObj);
		}

	}
}

void RADIO_SetTextColor(RADIO_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		RADIO_Obj *pObj = (RADIO_Obj *)hObj;
		if (Color != pObj->TextColor) {
			pObj->TextColor = Color;
			if (GUI_ARRAY_GetNumItems(&pObj->TextArray)) {
				WM_Invalidate(hObj);
			}
		}

	}
}

#define RADIO_BKCOLOR0_DEFAULT RGB_GRAYL(0xc0)           /* Inactive color */
#define RADIO_BKCOLOR1_DEFAULT RGB_WHITE          /* Active color */


/* Colors */
static const RGB_COLOR _aColorDisabled[] = { RGB_GRAYL(0xC0), RGB_GRAYL(0x80), RGB_BLACK, RADIO_BKCOLOR0_DEFAULT };
static const RGB_COLOR _aColorEnabled[] = { RGB_GRAYL(0xC0), RGB_GRAYL(0x80), RGB_BLACK, RADIO_BKCOLOR1_DEFAULT };
static const RGB_COLOR _ColorsCheck[] = { RGB_WHITE, RGB_BLACK };
/* Palettes */
static const GUI_LOGPALETTE _PalRadioDisabled = {
  4,	/* number of entries */
  1, 	/* Transparency */
  _aColorDisabled
};
static const GUI_LOGPALETTE _PalRadioEnabled = {
  4,	/* number of entries */
  1, 	/* Transparency */
  _aColorEnabled
};
static const GUI_LOGPALETTE _PalCheck = {
  2,	/* number of entries */
  1, 	/* Transparency */
  &_ColorsCheck[0]
};

/* Pixel data */
static const uint8_t _acRadio[] = {
________,XXXXXXXX,________,
____XXXX,oooooooo,XXXX____,
__XXoooo,dddddddd,oooodd__,
__XXoodd,dddddddd,dd__dd__,
XXoodddd,dddddddd,dddd__dd,
XXoodddd,dddddddd,dddd__dd,
XXoodddd,dddddddd,dddd__dd,
XXoodddd,dddddddd,dddd__dd,
__XXoodd,dddddddd,dd__dd__,
__XX____,dddddddd,____dd__,
____dddd,________,dddd____,
________,dddddddd,________,
};
static const uint8_t _acCheck[] = {
  __XXXX__________,
  XXXXXXXX________,
  XXXXXXXX________,
  __XXXX__________
};
/* Bitmaps */
const GUI_BITMAP RADIO__abmRadio[] = {
  { 12, 12, 3, 2, _acRadio, &_PalRadioDisabled},
  { 12, 12, 3, 2, _acRadio, &_PalRadioEnabled}
};

const GUI_BITMAP RADIO__bmCheck = {
  4, /* XSize */
  4, /* YSize */
  1, /* BytesPerLine */
  1, /* BitsPerPixel */
  _acCheck,  /* Pointer to picture data (indices) */
  &_PalCheck  /* Pointer to palette */
};
