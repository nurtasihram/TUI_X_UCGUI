module;

#include "GUI.h"

export module TUX.Widget.Radio;

import TUX.Widget;

import TUX.Array;

/* Define default background color */
#define RADIO_DEFAULT_BKCOLOR       RGB_GRAYL(0xC0)
#define RADIO_BORDER                2

extern CBITMAP _abmRadio[2];
extern CBITMAP _bmCheck;

export {

constexpr uint16_t
	RADIO_TEXTPOS_RIGHT = 0,
	RADIO_TEXTPOS_LEFT  = WIDGET_STATE_USER<0>; /* Not implemented, TBD */

enum RADIO_CI {
	 RADIO_BI_INACTIV = 0,
	 RADIO_BI_ACTIV,
	 RADIO_BI_CHECK
};

class Radio : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		RGBC TextColor{ RGB_BLACK };
		RGBC BkColor{ RGB_INVALID };
		PCBITMAP apBmRadio[2]{ &_abmRadio[0], &_abmRadio[1] };
		PCBITMAP pBmCheck{ &_bmCheck };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	ARRAY<char *> TextArray;
	int16_t Sel = -1;
	uint16_t Spacing;
	uint16_t NumItems;
	uint16_t Height = Props.apBmRadio[0]->Size.y + RADIO_BORDER * 2;
	uint8_t  GroupId = 0;

	void _OnPaint() const {
		/* Init some data */
		auto rFocus = GetClientRect();
		bool HasFocus = States & WIDGET_STATE_FOCUS;
		auto pBmRadio = Props.apBmRadio[IsEnabled()],
			 pBmCheck = Props.pBmCheck;
		rFocus.x1 = pBmRadio->Size.x + RADIO_BORDER * 2 - 1;
		rFocus.y1 = Height + ((NumItems - 1) * Spacing) - 1;
		/* Select font and text color */
		GUI.Color(Props.TextColor);
		GUI.Font(Props.pFont);
		GUI.SetTextMode(DRAWMODE_TRANS);
		auto FontDistY = Props.pFont->YSize;
		auto CHeight = FontDistY;
		RECT Rect;
		Rect.x0 = pBmRadio->Size.x + RADIO_BORDER * 2 + 2;
		Rect.y0 = CHeight <= Height ? (Height - CHeight) / 2 : 0;
		Rect.y1 = Rect.y0 + CHeight - 1;
		auto FocusBorder = (FontDistY <= 12) ? 2 : 3;
		if (Rect.y0 < FocusBorder)
			FocusBorder = Rect.y0;
		/* Clear inside ... Just in case      */
		/* Fill with parents background color */
		SetBkColorPrefer(Props.BkColor);
		GUI_Clear();
		/* Iterate over all items */
		for (int i = 0; i < NumItems; i++) {
			auto y = i * Spacing;
			/* Draw the radio button bitmap */
			GUI_DrawBitmap(pBmRadio, { RADIO_BORDER, RADIO_BORDER + y });
			/* Draw the check bitmap */
			if (Sel == i)
				GUI_DrawBitmap(pBmCheck, {
					RADIO_BORDER + (pBmRadio->Size.x - pBmCheck->Size.x) / 2,
					RADIO_BORDER + ((pBmRadio->Size.y - pBmCheck->Size.y) / 2) + y });
			/* Draw text if available */
			if (auto pText = TextArray[i]) {
				auto r = Rect;
				r.x1 = r.x0 + GUI_GetStringSizeX(pText) - 2;
				r += POINT{ 0, y };
				GUI_DispStringAt(pText, r.x0, r.y0);
				/* Calculate focus rect */
				if (HasFocus && Sel == i)
					rFocus = r * FocusBorder;
			}
		}
		/* Draw the focus rect */
		if (HasFocus) {
			GUI.Color(RGB_BLACK);
			GUI_DrawFocusRect(rFocus, 0);
		}
	}
	void _OnTouch(const PID_STATE *pState) {
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
				if (IsFocussable()) {
					SetFocus();
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
		NotifyParent(Notification);
		if (Hit == 1) {
			GUI_StoreKey(GetId());
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

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Radio *)pWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_GET_RADIOGROUP:
				return pObj->GroupId;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
			case WM_DELETE:
				for (int i = 0; i < pObj->TextArray.NumItems(); i++)
					GUI__SetText(pObj->TextArray[i], nullptr);
				pObj->TextArray.Delete();
				return 0;
		}
		return DefaultProc(pWin, MsgId, Data);
	}

private:
	static void _AdjRect(RECT &r, uint16_t NumItems, uint16_t Spacing) {
		auto Height = DefaultProps.apBmRadio[0]->Size.y + RADIO_BORDER * 2;
		if (r.x1 <= r.x0)
			r.x1 += DefaultProps.apBmRadio[0]->Size.x + RADIO_BORDER * 2;
		if (r.y1 <= r.y0)
			r.y1 += Height + (NumItems - 1) * Spacing;
	}
public:
	Radio(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
		  uint16_t ExFlags, uint16_t NumItems, uint16_t Spacing) :
		Widget((_AdjRect(r, NumItems, Spacing), r), Style, _Callback, pParent, Id, ExFlags | WIDGET_STATE_FOCUSSABLE),
		Spacing(Spacing ? Spacing : 20),
		NumItems(NumItems ? NumItems : 2) {
		for (int i = 0; i < NumItems; i++)
			TextArray.AddItem();
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		uint16_t NumItems = (pCreateInfo->Para) & 0xFF;
		uint16_t Spacing = (pCreateInfo->Para >> 8) & 0xFF;
		return new Radio(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, pWinParent, pCreateInfo->Id,
			0, NumItems, Spacing);
	}

private:
	void _SetValue(int v) {
		if (v >= NumItems) {
			v = NumItems - 1;
		}
		if (Sel != v) {
			Sel = v;
			Invalidate();
			NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	static int _IsInGroup(WObj *pWin, uint8_t GroupId) {
		if (GroupId)
			return pWin->Require(WM_GET_RADIOGROUP, 0) == GroupId;
		return 0;
	}
	static Radio *_GetPrevInGroup(WObj *pWin, uint8_t GroupId) {
		for (pWin = pWin->PrevSibling(); pWin; pWin = pWin->PrevSibling())
			if (_IsInGroup(pWin, GroupId))
				return (Radio *)pWin;
		return nullptr;
	}
	static Radio *_GetNextInGroup(WObj *pWin, uint8_t GroupId) {
		for (; pWin; pWin = pWin->NextSibling())
			if (_IsInGroup(pWin, GroupId))
				return (Radio *)pWin;
		return nullptr;
	}
	void _ClearSelection(uint8_t GroupId) {
		for (auto pWin = FirstSibling(); pWin; pWin = pWin->NextSibling()) {
			if (pWin != this)
				if (_IsInGroup(pWin, GroupId))
					((Radio *)pWin)->_SetValue(-1);
		}
	}
	void _HandleSetValue(int v) {
		if (v < 0) {
			auto pWin = _GetPrevInGroup(this, GroupId);
			if (pWin) {
				pWin->SetFocus();
				pWin->_SetValue(0x7FFF);
				_SetValue(-1);
			}
		}
		else if (v >= NumItems) {
			auto pWin = _GetNextInGroup(this, GroupId);
			if (pWin) {
				pWin->SetFocus();
				pWin->_SetValue(0);
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

public:

#pragma region Properties

	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}

	void TextColor(RGBC Color) {
		if (Props.TextColor == Color)
			return;
		Props.TextColor = Color;
		Invalidate();
	}

	void BkColor(RGBC Color) {
		if (Props.BkColor == Color)
			return;
		Props.BkColor = Color;
		Invalidate();
	}

#pragma endregion

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

	void SetGroupId(uint8_t NewGroupId) {
		auto OldGroupId = GroupId;
		if (NewGroupId != OldGroupId) {
			auto pFirst = FirstSibling();
			/* Pass our selection, if we have one, to another radio button in */
			/* our old group. So the group have a valid selection when we leave it. */
			if (OldGroupId && Sel >= 0) {
				GroupId = 0; /* Leave group first, so _GetNextInGroup() could */
				/* not find a handle to our own window. */
				if (auto pWin = _GetNextInGroup(pFirst, OldGroupId))
					pWin->_SetValue(0);
			}
			/* Make sure we have a valid selection according to our new group */
			if (_GetNextInGroup(pFirst, NewGroupId) != 0) {
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
		Invalidate();
	}
	void SetText(const char *pText, unsigned Index) {
		if (Index < (unsigned)NumItems) {
			GUI__SetText(TextArray[Index], pText);
			Invalidate();
		}
	}


};

Radio::Properties Radio::DefaultProps;

}

#define RADIO_BKCOLOR0_DEFAULT RGB_GRAYL(0xc0)           /* Inactive color */
#define RADIO_BKCOLOR1_DEFAULT RGB_WHITE          /* Active color */

static const uint8_t _pxRadio[]{
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
static CLOGPALETTE _PalRadioDisabled{ RGB_INVALID, RGB_GRAYL(0x80), RGB_BLACK, RADIO_BKCOLOR0_DEFAULT };
static CLOGPALETTE _PalRadioEnabled{ RGB_INVALID, RGB_GRAYL(0x80), RGB_BLACK, RADIO_BKCOLOR1_DEFAULT };
CBITMAP _abmRadio[]{
	{ 12, 3, 2, _pxRadio, _PalRadioDisabled },
	{ 12, 3, 2, _pxRadio, _PalRadioEnabled }
};

static const uint8_t _pxCheck[]{
__XXXX__________,
XXXXXXXX________,
XXXXXXXX________,
__XXXX__________
};
static CLOGPALETTE _PalCheck{ RGB_INVALID, RGB_BLACK };
CBITMAP _bmCheck{ 4, 1, 1, _pxCheck, _PalCheck };
