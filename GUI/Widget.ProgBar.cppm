export module TUX.Widget.ProgBar;

#include "GUIConf.h"

import TUX.Widget;

export {

enum PROGBAR_CI {
	 PROGBAR_CI_ACTIVE = 0,
	 PROGBAR_CI_INACT
};

class ProgBar : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		BRUSH aBrush[2]{
			/* Index       | Background      | Text         */
			/* Active   */ { RGB_DARKBLUE    , RGB_WHITE    },
			/* Inactive */ { RGBC::Gray(0x55), RGB_BLACK    },
		};
		TEXTALIGN Align{ TEXTALIGN_CENTER };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	char *pText = nullptr;
	int16_t XOff = 0, YOff = 0;
	int16_t v = 0, Min = 0, Max = 0;

	void _FreeText() {
		GUI_MEM_FreePtr((void **)&pText);
		pText = nullptr;
	}
	auto _Value2X(int16_t v) const {
		auto EffectSize = this->EffectSize();
		auto xSize = SizeX();
		if (v < Min)
			v = Min;
		if (v > Max)
			v = Max;
		return EffectSize + ((xSize - 2 * EffectSize) * (v - Min)) / (Max - Min);
	}
	void _DrawPart(PROGBAR_CI Index, const RECT &rText, const char *pText) const {
		GUI.Brush(Props.aBrush[Index]);
		GUI.Clear();
		GUI_DispStringInRect(pText, rText, Props.Align);
	}
	const char *_GetText(char *pBuffer) const {
		if (pText) return pText;
		auto pText = pBuffer;
		uint8_t value = 100 * (v - Min) / (Max - Min);
		if (value == 100) {
			*pBuffer++ = '1';
			*pBuffer++ = '0';
			*pBuffer++ = '0';
		}
		else {
			if (value >= 10) {
				*pBuffer++ = '0' + value / 10;
				value %= 10;
			}
			*pBuffer++ = '0' + value;
		}
		*pBuffer++ = '%';
		*pBuffer = '\0';
		return pText;
	}
	void _OnPaint() const {
		auto rClient = ClientRect();
		auto rInside = rClient / EffectSize();
		auto xPos = _Value2X(v);
		char ac[5]{ 0 };
		auto pText = _GetText(ac);
		GUI.Font(Props.pFont);
		/* Draw left bar */
		auto r = rInside;
		r.x1 = xPos - 1;
		UserClip(&r);
		_DrawPart(PROGBAR_CI_INACT, rClient, pText);
		/* Draw right bar */
		r = rInside;
		r.x0 = xPos;
		UserClip(&r);
		_DrawPart(PROGBAR_CI_ACTIVE, rClient, pText);
		UserClip(nullptr);
		DrawDown(rClient);
	}
	void _Delete() {
		_FreeText();
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (ProgBar *)pWin;
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
	ProgBar(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget(r, Style, _Callback, pParent, Id, 0) {
		SetEffect(WIDGET_Effect_None); /* Standard effect for progbar: None */
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new ProgBar(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, pWinParent, pCreateInfo->Id);
	}

public:

#pragma region Properties
	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}
	void Brush(PROGBAR_CI Index, BRUSH aBrush) {
		if (Props.aBrush[Index] == aBrush)
			return;
		Props.aBrush[Index] = aBrush;
		Invalidate();
	}
	void TextAlign(TEXTALIGN Align) {
		if (Props.Align == Align)
			return;
		Props.Align = Align;
		Invalidate();
	}
#pragma endregion

	void SetValue(int v) {
		/* Put v into legal range */
		if (v < Min)
			v = Min;
		if (v > Max)
			v = Max;
		if (this->v != v) {
			this->v = v;
			Invalidate();
		}
	}
	void SetText(const char *s) {
		if (GUI__SetText(pText, s))
			Invalidate();
	}
	void SetTextPos(int XOff, int YOff) {
		this->XOff = XOff;
		this->YOff = YOff;
		Invalidate();
	}
	void SetMinMax(int Min, int Max) {
		if (Max > Min) {
			if (Max != this->Max || Min != this->Min) {
				this->Min = Min;
				this->Max = Max;
				Invalidate();
			}
		}
	}
};

ProgBar::Properties ProgBar::DefaultProps;

}
