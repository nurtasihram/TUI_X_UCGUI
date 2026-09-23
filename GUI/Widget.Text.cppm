export module TUX.Widget.Text;

#include "GUIConf.h"

import TUX.Widget;

export {

constexpr uint16_t
	TEXT_CF_LEFT    = TEXTALIGN_LEFT,
	TEXT_CF_RIGHT   = TEXTALIGN_RIGHT,
	TEXT_CF_HCENTER = TEXTALIGN_HCENTER,
	TEXT_CF_VCENTER = TEXTALIGN_VCENTER,
	TEXT_CF_TOP     = TEXTALIGN_TOP,
	TEXT_CF_BOTTOM  = TEXTALIGN_BOTTOM;

class Text : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		BRUSH brush{ RGBC::Gray(0xE4), RGB_BLACK };
		TEXTALIGN Align{ TEXTALIGN_LEFT | TEXTALIGN_TOP };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	char *pText = nullptr;

	void _OnPaint() const {
		GUI.Font(Props.pFont);
		GUI.Brush(Props.brush);
		GUI.Clear();
		GUI_DispStringInRect(pText, ClientRect(), Props.Align);
	}
	void _Delete() {
		GUI_MEM_FreePtr((void **)&pText);
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Text *)pWin;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE:
				pObj->_Delete();
				return 0;
		}
		return pObj->WidgetProc(MsgId, Data);
	}

public:
	Text(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
		 TEXTALIGN ExFlags, const char *pText) :
		Widget(r, Style, _Callback, pParent, Id, 0) {
		if (pText)
			GUI__SetText(this->pText, pText);
		else
			this->pText = nullptr;
		Props.Align = ExFlags;
	}

	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new Text(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			WC_VISIBLE, pWinParent, pCreateInfo->Id,
			(TEXTALIGN)pCreateInfo->Flags, pCreateInfo->pName);
	}

public:

#pragma region properties
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

	auto TextAlign() const { return Props.Align; }
	void TextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}
#pragma endregion

	void SetText(const char *s) {
		if (GUI__SetText(pText, s))
			Invalidate();
	}
};

Text::Properties Text::DefaultProps;

}
