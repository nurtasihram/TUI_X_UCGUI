export module TUX.Widget.Radio;

#include "GUIConf.h"

import TUX.Widget;

import TUX.Array;

/* Define default background color */
#define RADIO_DEFAULT_BKCOLOR       RGBC::Gray(0xC0)
#define RADIO_BORDER                2

extern CBITMAP _abmRadio[2];
extern CBITMAP _bmCheck;

export {

constexpr uint16_t
	RADIO_TEXTPOS_RIGHT = 0,
	RADIO_TEXTPOS_LEFT  = WIDGET_STATE_USER<0>; /* Not implemented, TBD */

enum RADIO_BI {
	 RADIO_BI_INACTIV = 0,
	 RADIO_BI_ACTIV,
	 RADIO_BI_CHECK
};

class Radio : public Widget {

public:
	struct Properties {
		PCBITMAP apBmRadio[2]{ &_abmRadio[0], &_abmRadio[1] };
		PCBITMAP pBmCheck{ &_bmCheck };
		PCFONT pFont{ GUI_DEFAULT_FONT };
		BRUSH brush{ RGB_INVALID, RGB_BLACK };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	ARRAY<char *> TextArray;
	int16_t Sel = -1;
	uint16_t Spacing;
	uint16_t Height = Props.apBmRadio[0]->Size.y + RADIO_BORDER * 2;
	uint8_t  GroupId = 0;

	void _OnPaint() const {
		/* Init some data */
		bool HasFocus = States & WIDGET_STATE_FOCUS;
		auto pBmRadio = Props.apBmRadio[IsEnabled()],
			 pBmCheck = Props.pBmCheck;
		auto NumItems = this->NumItems();
		auto rFocus = ClientRect();
		rFocus.x1 = pBmRadio->Size.x + RADIO_BORDER * 2 - 1;
		rFocus.y1 = Height + (NumItems - 1) * Spacing - 1;
		/* Select font and text color */
		GUI.Brush(Props.brush);
		auto FontDistY = Props.pFont->YSize;
		RECT r;
		r.x0 = pBmRadio->Size.x + RADIO_BORDER * 2 + 2;
		r.y0 = FontDistY <= Height ? (Height - FontDistY) / 2 : 0;
		r.y1 = r.y0 + FontDistY - 1;
		auto FocusBorder = FontDistY <= 12 ? 2 : 3;
		if (r.y0 < FocusBorder)
			FocusBorder = r.y0;
		/* Clear inside ... Just in case      */
		/* Fill with parents background color */
		SetBkColorPrefer(Props.brush.BkColor);
		GUI.Clear();
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
				auto rText = r;
				rText.x1 = rText.x0 + Props.pFont->TextBound(pText).x - 2;
				rText += POINT{ 0, y };
				GUI_DispStringAt(pText, rText.x0, rText.y0);
				/* Calculate focus rect */
				if (HasFocus && Sel == i)
					rFocus = rText * FocusBorder;
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
				auto y = pState->y;
				auto Sel = y / Spacing;
				y -= Sel * Spacing;
				if (y <= Height)
					SetValue(Sel);
				if (IsFocussable())
					SetFocus();
				Notification = WM_NOTIFICATION_CLICKED;
			}
			else {
				Hit = 1;
				Notification = WM_NOTIFICATION_RELEASED;
			}
		}
		else
			Notification = WM_NOTIFICATION_MOVED_OUT;
		NotifyParent(Notification);
	}
	char _OnKey(const KEY_STATE *pInfo) {
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
				if (pObj->_OnKey((const KEY_STATE *)Data))
					return 0;
				break;
			case WM_DELETE:
				for (int i = 0; i < pObj->TextArray.NumItems(); i++)
					GUI__SetText(pObj->TextArray[i], nullptr);
				pObj->TextArray.Delete();
				return 0;
		}
		return pObj->WidgetProc(MsgId, Data);
	}

private:
	static void _AdjRect(RECT &r, uint16_t NumItems, uint16_t Spacing) {
		if (r.x1 <= r.x0)
			r.x1 += DefaultProps.apBmRadio[0]->Size.x + RADIO_BORDER * 2;
		if (r.y1 <= r.y0)
			r.y1 += DefaultProps.apBmRadio[0]->Size.y + RADIO_BORDER * 2 + (NumItems - 1) * Spacing;
	}
public:
	Radio(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
		  uint16_t ExFlags, uint16_t NumItems, uint16_t Spacing = 0) :
		Widget((_AdjRect(r, NumItems, Spacing), r), Style, _Callback, pParent, Id, ExFlags | WIDGET_STATE_FOCUSSABLE),
		Spacing(Spacing ? Spacing : 20) {
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
		if (v >= NumItems())
			v = NumItems() - 1;
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
			if (auto pWin = _GetPrevInGroup(this, GroupId)) {
				pWin->SetFocus();
				pWin->_SetValue(0x7FFF);
				_SetValue(-1);
			}
		}
		else if (v >= NumItems()) {
			if (auto pWin = _GetNextInGroup(this, GroupId)) {
				pWin->SetFocus();
				pWin->_SetValue(0);
				_SetValue(-1);
			}
		}
		else if (Sel != v) {
			_ClearSelection(GroupId);
			_SetValue(v);
		}
	}

public:

#pragma region Properties
	auto Font() const { return Props.pFont; }
	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}

	auto Brush() const { return Props.brush; }
	void Brush(BRUSH brush) {
		if (Props.brush == brush)
			return;
		Props.brush = brush;
		Invalidate();
	}

	void SetImage(RADIO_BI Index, PCBITMAP pBitmap) {
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
#pragma endregion

	void Dec() { SetValue(Sel - 1); }
	void Inc() { SetValue(Sel + 1); }
	auto GetValue() const { return Sel; }
	void SetValue(int v) {
		if (GroupId)
			_HandleSetValue(v);
		else {
			if (v < 0)
				v = 0;
			_SetValue(v);
		}
	}

	void SetGroupId(uint8_t NewGroupId) {
		if (GroupId == NewGroupId)
			return;
		auto OldGroupId = GroupId;
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
		if (_GetNextInGroup(pFirst, NewGroupId))
			/* Join an existing group with an already valid selection, so clear our own one */
			_SetValue(-1);
		else if (Sel < 0)
			/* We are the first window in group, so we must have a valid selection at our own. */
			_SetValue(0);
		/* Change the group */
		GroupId = NewGroupId;
	}

	auto NumItems() const { return TextArray.NumItems(); }
	void SetText(uint16_t Index, const char *pText) {
		if (Index < NumItems()) {
			GUI__SetText(TextArray[Index], pText);
			Invalidate();
		}
	}
};

Radio::Properties Radio::DefaultProps;

}

#define RADIO_BKCOLOR0_DEFAULT RGBC::Gray(0xc0)           /* Inactive color */
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
static CLOGPALETTE _PalRadioDisabled{ RGB_INVALID, RGBC::Gray(0x80), RGB_BLACK, RADIO_BKCOLOR0_DEFAULT };
static CLOGPALETTE _PalRadioEnabled{ RGB_INVALID, RGBC::Gray(0x80), RGB_BLACK, RADIO_BKCOLOR1_DEFAULT };
CBITMAP _abmRadio[]{
	{ 12, 3, BPP_2, _pxRadio, _PalRadioDisabled },
	{ 12, 3, BPP_2, _pxRadio, _PalRadioEnabled }
};

static const uint8_t _pxCheck[]{
__XXXX__________,
XXXXXXXX________,
XXXXXXXX________,
__XXXX__________
};
static CLOGPALETTE _PalCheck{ RGB_INVALID, RGB_BLACK };
CBITMAP _bmCheck{ 4, 1, BPP_1, _pxCheck, _PalCheck };
