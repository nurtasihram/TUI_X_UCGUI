module;

#include "GUIDebug.h"      /* Req. for GUI_DEBUG_LEVEL */
#include "DIALOG_Intern.h" /* Req. for Create indirect data structure */

export module TUX.Widget.Text;

import TUX.Widget;

export {
constexpr uint16_t TEXT_CF_LEFT    = TEXTALIGN_LEFT;
constexpr uint16_t TEXT_CF_RIGHT   = TEXTALIGN_RIGHT;
constexpr uint16_t TEXT_CF_HCENTER = TEXTALIGN_HCENTER;
constexpr uint16_t TEXT_CF_VCENTER = TEXTALIGN_VCENTER;
constexpr uint16_t TEXT_CF_TOP     = TEXTALIGN_TOP;
constexpr uint16_t TEXT_CF_BOTTOM  = TEXTALIGN_BOTTOM;

class Text : public WIDGET {

public:
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		TEXTALIGN Align{ 0 };
		RGBC TextColor{ RGB_BLACK };
		RGBC BkColor{ RGB_INVALID_COLOR };
	} static DefaultProps;
	
private:
	Properties Props;

	char *pText;

	void _FreeAttached() {
		GUI_ALLOC_FreePtr((void **)&pText);
	}
	void _OnPaint() {
		const char *s;
		GUI.SetColor(Props.TextColor);
		GUI_SetFont(Props.pFont);
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
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
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

	static Text *Create(int x0, int y0, int xsize, int ysize, WObj *hParent,
						int WinFlags, int ExFlags, int Id, const char *pText) {
		/* Create the window */
#if WM_SUPPORT_TRANSPARENCY
		WinFlags |= WC_HASTRANS;
#endif
		auto pObj = (Text *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, Text::_Callback,
												   sizeof(Text) - sizeof(WObj));
		if (!pObj) {
			GUI_DEBUG_ERROROUT_IF(pObj == 0, "Text create failed");
			return nullptr;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, 0);
		/* init member variables */
		pObj->Props = Text::DefaultProps;
		pObj->Props.Align = ExFlags;
		if (pText)
			GUI__SetText(&pObj->pText, pText);
		else
			pObj->pText = nullptr;
		return pObj;
	}
	static WObj *CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return Create(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
					  hWinParent, WC_VISIBLE, pCreateInfo->Flags, pCreateInfo->Id, pCreateInfo->pName);
	}

public:
	void SetBkColor(RGBC Color) {
		Props.BkColor = Color;
		WM_Invalidate(this);
	}
	void SetFont(PCFONT pFont) {
		Props.pFont = pFont;
		WM_Invalidate(this);
	}
	void SetText(const char *s) {
		if (GUI__SetText(&pText, s))
			WM_Invalidate(this);
	}
	void SetTextAlign(int Align) {
		Props.Align = Align;
		WM_Invalidate(this);
	}
	void SetTextColor(RGBC Color) {
		Props.TextColor = Color;
		WM_Invalidate(this);
	}

};

Text::Properties Text::DefaultProps;

}
