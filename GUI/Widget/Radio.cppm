module;

#include "DIALOG_Intern.h"

export module TUX.Widget.Radio;

import TUX.Widget;

import TUX.Array;

/* Define default background color */
#define RADIO_DEFAULT_BKCOLOR       RGB_GRAYL(0xC0)
#define RADIO_BORDER                  2

extern CBITMAP _abmRadio[2];
extern CBITMAP _bmCheck;

export {
enum RADIO_CI {
	 RADIO_BI_INACTIV = 0,
	 RADIO_BI_ACTIV   = 1,
	 RADIO_BI_CHECK   = 2
};

constexpr uint16_t RADIO_TEXTPOS_RIGHT       = 0;
constexpr uint16_t RADIO_TEXTPOS_LEFT        = WIDGET_STATE_USER0;/* Not implemented, TBD */

struct RADIO_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC TextColor{ RGB_BLACK };
		RGBC BkColor{ RGB_INVALID_COLOR };
		PCBITMAP apBmRadio[2]{ &_abmRadio[0], &_abmRadio[1] };
		PCBITMAP pBmCheck{ &_bmCheck };
	} static DefaultProps;
	Properties Props;
	GUI_ARRAY TextArray;
	int16_t Sel; /* current selection */
	uint16_t Spacing;
	uint16_t Height;
	uint16_t NumItems;
	uint8_t  GroupId;

	void _ResizeRect(GUI_RECT *pDest, const GUI_RECT *pSrc, int Diff) {
		pDest->y0 = pSrc->y0 - Diff;
		pDest->y1 = pSrc->y1 + Diff;
		pDest->x0 = pSrc->x0 - Diff;
		pDest->x1 = pSrc->x1 + Diff;
	}
	void _OnPaint() {
		PCBITMAP pBmRadio;
		PCBITMAP pBmCheck;
		const char *pText;
		GUI_RECT Rect, r, rFocus;
		int i, y, HasFocus, FontDistY;
		uint16_t SpaceAbove, CHeight, FocusBorder;
		/* Init some data */
		rFocus = WIDGET__GetClientRect(this);
		HasFocus = (this->State & WIDGET_STATE_FOCUS) ? 1 : 0;
		pBmRadio = this->Props.apBmRadio[WM_IsEnabled(this)];
		pBmCheck = this->Props.pBmCheck;
		rFocus.x1 = pBmRadio->XSize + RADIO_BORDER * 2 - 1;
		rFocus.y1 = this->Height + ((this->NumItems - 1) * this->Spacing) - 1;
		/* Select font and text color */
		GUI_SetColor(this->Props.TextColor);
		GUI_SetFont(this->Props.pFont);
		GUI_SetTextMode(DRAWMODE_TRANS);
		FontDistY = GUI_GetFontDistY();
		CHeight = this->Props.pFont->CHeight;
		SpaceAbove = this->Props.pFont->Baseline - CHeight;
		Rect.x0 = pBmRadio->XSize + RADIO_BORDER * 2 + 2;
		Rect.y0 = (CHeight <= this->Height) ? ((this->Height - CHeight) / 2) : 0;
		Rect.y1 = Rect.y0 + CHeight - 1;
		FocusBorder = (FontDistY <= 12) ? 2 : 3;
		if (Rect.y0 < FocusBorder) {
			FocusBorder = Rect.y0;
		}
		/* Clear inside ... Just in case      */
		/* Fill with parents background color */
#if WM_SUPPORT_TRANSPARENCY
		if (!WM_GetHasTrans(this))
#endif
		{
			if (this->Props.BkColor != RGB_INVALID_COLOR) {
				GUI_SetBkColor(this->Props.BkColor);
			}
			else {
				GUI_SetBkColor(RADIO_DEFAULT_BKCOLOR);
			}
			GUI_Clear();
		}
		/* Iterate over all items */
		for (i = 0; i < this->NumItems; i++) {
			y = i * this->Spacing;
			/* Draw the radio button bitmap */
			GUI_DrawBitmap(pBmRadio, RADIO_BORDER, RADIO_BORDER + y);
			/* Draw the check bitmap */
			if (this->Sel == i) {
				GUI_DrawBitmap(pBmCheck, RADIO_BORDER + (pBmRadio->XSize - pBmCheck->XSize) / 2,
							   RADIO_BORDER + ((pBmRadio->YSize - pBmCheck->YSize) / 2) + y);
			}
			/* Draw text if available */
			pText = (const char *)GUI_ARRAY_GetpItem(&this->TextArray, i);
			if (pText) {
				if (*pText) {
					r = Rect;
					r.x1 = r.x0 + GUI_GetStringDistX(pText) - 2;
					r += GUI_POINT{ 0, y };
					GUI_DispStringAt(pText, r.x0, r.y0 - SpaceAbove);
					/* Calculate focus rect */
					if (HasFocus && (this->Sel == i)) {
						_ResizeRect(&rFocus, &r, FocusBorder);
					}
				}
			}
		}
		/* Draw the focus rect */
		if (HasFocus) {
			GUI_SetColor(RGB_BLACK);
			WIDGET__DrawFocusRect(this, rFocus, 0);
		}
	}
	void _OnTouch(const GUI_PID_STATE *pState) {
		int Notification;
		int Hit = 0;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				int y, Sel;
				y = pState->y;
				Sel = y / this->Spacing;
				y -= Sel * this->Spacing;
				if (y <= this->Height) {
					SetValue( Sel);
				}
				if (WM_IsFocussable(this)) {
					WM_SetFocus(this);
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
		WM_NotifyParent(this, Notification);
		if (Hit == 1) {
			GUI_StoreKey(this->Id);
		}
	}
	char _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) {
			switch (pInfo->Key) {
				case GUI_KEY_RIGHT:
				case GUI_KEY_DOWN:
					Inc();
					return 1;
				case GUI_KEY_LEFT:
				case GUI_KEY_UP:
					Dec();
					return 1;
			}
		}
		return 0;
	}

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (RADIO_Obj *)hWin;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_GET_RADIOGROUP:
				return pObj->GroupId;
			case WM_TOUCH:
				pObj->_OnTouch((const GUI_PID_STATE *)Data);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
			case WM_DELETE:
				GUI_ARRAY_Delete(&pObj->TextArray);
				return 0;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:

	void _SetValue(int v) {
		if (v >= NumItems) {
			v = NumItems - 1;
		}
		if (Sel != v) {
			Sel = v;
			WM_Invalidate(this);
			WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	static int _IsInGroup(WM_Obj *pWin, uint8_t GroupId) {
		if (GroupId)
			return WM_SendMessage(pWin, WM_GET_RADIOGROUP, 0) == GroupId;
		return 0;
	}
	static RADIO_Obj *_GetPrevInGroup(WM_Obj *pWin, uint8_t GroupId) {
		for (pWin = WM_GetPrevSibling(pWin); pWin; pWin = WM_GetPrevSibling(pWin))
			if (_IsInGroup(pWin, GroupId))
				return (RADIO_Obj *)pWin;
		return nullptr;
	}
	static RADIO_Obj *_GetNextInGroup(WM_Obj *pWin, uint8_t GroupId) {
		for (; pWin; pWin = WM_GetNextSibling(pWin))
			if (_IsInGroup(pWin, GroupId))
				return (RADIO_Obj *)pWin;
		return nullptr;
	}
	void _ClearSelection(uint8_t GroupId) {
		for (auto pWin = (WM_Obj *)WM__GetFirstSibling(this); pWin; pWin = pWin->pNext) {
			if (pWin != this)
				if (_IsInGroup(pWin, GroupId))
					((RADIO_Obj *)pWin)->_SetValue(-1);
		}
	}
	void _HandleSetValue(int v) {
		if (v < 0) {
			auto hWin = _GetPrevInGroup(this, GroupId);
			if (hWin) {
				WM_SetFocus(hWin);
				hWin->_SetValue(0x7FFF);
				_SetValue(-1);
			}
		}
		else if (v >= NumItems) {
			auto hWin = _GetNextInGroup(this, GroupId);
			if (hWin) {
				WM_SetFocus(hWin);
				hWin->_SetValue(0);
				_SetValue(-1);
			}
		}
		else {
			if (Sel != v) {
				_ClearSelection(GroupId);
				_SetValue(v);
			}
		}
	}

	void AddValue(int Add) {
		SetValue(Sel + Add);
	}
	void Dec() {
		AddValue(-1);
	}
	void Inc() {
		AddValue(1);
	}
	void SetValue(int v) {
		if (GroupId) {
			_HandleSetValue(v);
		}
		else {
			if (v < 0) {
				v = 0;
			}
			_SetValue(v);
		}
	}
	int  GetValue() {
		return Sel;
	}
	void SetBkColor(RGBC Color) {
		if (Color != Props.BkColor) {
			Props.BkColor = Color;
#if WM_SUPPORT_TRANSPARENCY
			if (Color <= RGB_WHITE) {
				WM_SetTransState(this, 0);
			}
			else {
				WM_SetTransState(this, WM_CF_HASTRANS);
			}
#endif
			WM_Invalidate(this);
		}
	}
	void SetFont(PCFONT pFont) {
		if (pFont != Props.pFont) {
			Props.pFont = pFont;
			if (GUI_ARRAY_GetNumItems(&TextArray))
				WM_Invalidate(this);
		}
	}
	void SetGroupId(uint8_t NewGroupId) {
		auto OldGroupId = GroupId;
		if (NewGroupId != OldGroupId) {
			auto hFirst = WM__GetFirstSibling(this);
			/* Pass our selection, if we have one, to another radio button in */
			/* our old group. So the group have a valid selection when we leave it. */
			if (OldGroupId && (Sel >= 0)) {
				GroupId = 0; /* Leave group first, so _GetNextInGroup() could */
				/* not find a handle to our own window. */
				auto hWin = _GetNextInGroup(hFirst, OldGroupId);
				if (hWin) {
					hWin->_SetValue(0);
				}
			}
			/* Make sure we have a valid selection according to our new group */
			if (_GetNextInGroup(hFirst, NewGroupId) != 0) {
				/* Join an existing group with an already valid selection, so clear our own one */
				_SetValue(-1);
			}
			else if (Sel < 0) {
				/* We are the first window in group, so we must have a valid selection at our own. */
				_SetValue(0);
			}
			/* Change the group */
			GroupId = NewGroupId;
		}
	}
	void SetImage(PCBITMAP pBitmap, unsigned int Index) {
		switch (Index) {
			case RADIO_BI_INACTIV:
			case RADIO_BI_ACTIV:
				Props.apBmRadio[Index] = pBitmap;
				break;
			case RADIO_BI_CHECK:
				Props.pBmCheck = pBitmap;
				break;
		}
		WM_Invalidate(this);
	}
	void SetText(const char *pText, unsigned Index) {
		if (Index < (unsigned)NumItems) {
			GUI_ARRAY_SetItem(&TextArray, Index, pText, pText ? (GUI__strlen(pText) + 1) : 0);
			WM_Invalidate(this);
		}
	}
	void SetTextColor(RGBC Color) {
		if (Color != Props.TextColor) {
			Props.TextColor = Color;
			if (GUI_ARRAY_GetNumItems(&TextArray)) {
				WM_Invalidate(this);
			}
		}
	}

};

RADIO_Obj::Properties RADIO_Obj::DefaultProps;

RADIO_Obj *RADIO_CreateEx(int x0, int y0, int xSize, int ySize, WM_Obj *hParent,
							int WinFlags, int ExFlags, int Id, int NumItems, int Spacing) {
	/* Calculate helper variables */
	auto Height = RADIO_Obj::DefaultProps.apBmRadio[0]->YSize + RADIO_BORDER * 2;
	Spacing = (Spacing <= 0) ? 20 : Spacing;
	NumItems = (NumItems <= 0) ? 2 : NumItems;
	if (ySize == 0) {
		ySize = Height + ((NumItems - 1) * Spacing);
	}
	if (xSize == 0) {
		xSize = RADIO_Obj::DefaultProps.apBmRadio[0]->XSize + RADIO_BORDER * 2;
	}
#if WM_SUPPORT_TRANSPARENCY
	WinFlags |= WM_CF_HASTRANS;
#endif
	/* Create the window */
	auto pObj = (RADIO_Obj *)WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent, WinFlags, RADIO_Obj::_Callback, sizeof(RADIO_Obj) - sizeof(WM_Obj));
	if (pObj) {
		for (int i = 0; i < NumItems; i++)
			GUI_ARRAY_AddItem(&pObj->TextArray, nullptr, 0);
		/* Init widget specific variables */
		ExFlags &= RADIO_TEXTPOS_LEFT;
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE | ExFlags);
		/* Init member variables */
		pObj->Props = RADIO_Obj::DefaultProps;
		pObj->Props.BkColor = WM_GetBkColor(hParent);
		pObj->NumItems = NumItems;
		pObj->Spacing = Spacing;
		pObj->Height = Height;
	}
	else {
	}
	return pObj;
}
RADIO_Obj *RADIO_Create(int x0, int y0, int xsize, int ysize, WM_Obj *hParent, int Id, int Flags, unsigned Para) {
	return RADIO_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id, Para & 0xFF, (Para >> 8) & 0xFF);
}
WM_Obj *RADIO_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	int NumItems = (pCreateInfo->Para) & 0xFF;
	int Spacing = (pCreateInfo->Para >> 8) & 0xFF;
	return RADIO_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
						   hWinParent, pCreateInfo->Flags, 0, pCreateInfo->Id, NumItems, Spacing);
}

}

#define RADIO_BKCOLOR0_DEFAULT RGB_GRAYL(0xc0)           /* Inactive color */
#define RADIO_BKCOLOR1_DEFAULT RGB_WHITE          /* Active color */

/* Colors */
static const RGBC _aColorDisabled[]{ RGB_GRAYL(0xC0), RGB_GRAYL(0x80), RGB_BLACK, RADIO_BKCOLOR0_DEFAULT };
static const RGBC _aColorEnabled[]{ RGB_GRAYL(0xC0), RGB_GRAYL(0x80), RGB_BLACK, RADIO_BKCOLOR1_DEFAULT };
static const RGBC _ColorsCheck[]{ RGB_WHITE, RGB_BLACK };
/* Palettes */
static const GUI_LOGPALETTE _PalRadioDisabled{
  4,	/* number of entries */
  1, 	/* Transparency */
  _aColorDisabled
};
static const GUI_LOGPALETTE _PalRadioEnabled{
  4,	/* number of entries */
  1, 	/* Transparency */
  _aColorEnabled
};
static const GUI_LOGPALETTE _PalCheck{
  2,	/* number of entries */
  1, 	/* Transparency */
  &_ColorsCheck[0]
};

/* Pixel data */
static const uint8_t _acRadio[]{
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
static const uint8_t _acCheck[]{
  __XXXX__________,
  XXXXXXXX________,
  XXXXXXXX________,
  __XXXX__________
};
/* Bitmaps */
CBITMAP _abmRadio[]{
  { 12, 12, 3, 2, _acRadio, &_PalRadioDisabled},
  { 12, 12, 3, 2, _acRadio, &_PalRadioEnabled}
};

CBITMAP _bmCheck{
  4, /* XSize */
  4, /* YSize */
  1, /* BytesPerLine */
  1, /* BitsPerPixel */
  _acCheck,  /* Pointer to picture data (indices) */
  &_PalCheck  /* Pointer to palette */
};
