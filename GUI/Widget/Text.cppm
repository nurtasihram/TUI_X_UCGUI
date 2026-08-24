module;

#include "GUI_Protected.h"

export module TUX.Widget.Text;

import TUX.Widget;

export {
constexpr uint16_t TEXT_CF_LEFT    = TEXTALIGN_LEFT;
constexpr uint16_t TEXT_CF_RIGHT   = TEXTALIGN_RIGHT;
constexpr uint16_t TEXT_CF_HCENTER = TEXTALIGN_HCENTER;
constexpr uint16_t TEXT_CF_VCENTER = TEXTALIGN_VCENTER;
constexpr uint16_t TEXT_CF_TOP     = TEXTALIGN_TOP;
constexpr uint16_t TEXT_CF_BOTTOM  = TEXTALIGN_BOTTOM;

class Text : public Widget {

public:
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		TEXTALIGN Align{ 0 };
		RGBC TextColor{ RGB_BLACK };
		RGBC BkColor{ RGB_INVALID_COLOR };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	char *pText = nullptr;

	void _FreeAttached() {
		GUI_ALLOC_FreePtr((void **)&pText);
	}
	void _OnPaint() {
		const char *s;
		GUI.SetColor(Props.TextColor);
		GUI.SetFont(Props.pFont);
		/* Fill with parents background color */
		SetBkColorPrefer(Props.BkColor);
		GUI_Clear();
		/* Show the text */
		if (pText) {
			s = pText;
			GUI.SetTextMode(DRAWMODE_TRANS);
			auto r = WM_GetClientRect();
			GUI_DispStringInRect(s, &r, Props.Align);
		}
	}
	void _Delete() {
		/* Delete attached objects (if any) */
		_FreeAttached();
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Text *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE:
				pObj->_Delete();
				return 0;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	Text(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
		 TEXTALIGN ExFlags, const char *pText) :
		Widget(r, Style, _Callback, pParent, Id, 0) {
		if (pText)
			GUI__SetText(&this->pText, pText);
		else
			this->pText = nullptr;
		Props.Align = ExFlags;
	}

	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new Text(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			WC_VISIBLE, hWinParent, pCreateInfo->Id,
			(TEXTALIGN)pCreateInfo->Flags, pCreateInfo->pName);
	}

public:

#pragma region Properties

	void SetFont(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}

	void SetBkColor(RGBC Color) {
		if (Props.BkColor == Color)
			return;
		Props.BkColor = Color;
		Invalidate();
	}

	void SetTextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}
	
	void SetTextColor(RGBC Color) {
		if (Props.TextColor == Color)
			return;
		Props.TextColor = Color;
		Invalidate();
	}

#pragma endregion

	void SetText(const char *s) {
		if (GUI__SetText(&pText, s))
			Invalidate();
	}

};

Text::Properties Text::DefaultProps;

}
