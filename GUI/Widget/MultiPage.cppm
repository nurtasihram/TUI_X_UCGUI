module;

#include "DIALOG_Intern.h"

export module TUX.Widget.MultiPage;

import TUX.Widget;
import TUX.Widget.ScrollBar;

import TUX.Array;

#define MULTIPAGE_STATE_ENABLED     (1<<0)
#define MULTIPAGE_STATE_SCROLLMODE  WIDGET_STATE_USER0
#define MULTIPAGE_NUMCOLORS         2

export {
constexpr uint16_t MULTIPAGE_ALIGN_LEFT   = 0 << 0;
constexpr uint16_t MULTIPAGE_ALIGN_RIGHT  = 1 << 0;
constexpr uint16_t MULTIPAGE_ALIGN_TOP    = 0 << 2;
constexpr uint16_t MULTIPAGE_ALIGN_BOTTOM = 1 << 2;

struct MULTIPAGE_PAGE {
	WM_Obj *hWin;
	uint8_t Status;
	char    acText;
};
struct MULTIPAGE_Obj : public WIDGET {
	struct Properties {
		PCFONT Font{ &FontProp13_1 };
		RGBC aBkColor[MULTIPAGE_NUMCOLORS]{
			/* Disabled page */	RGB_GRAYL(0xD0),
			/* Enabled page */	RGB_GRAYL(0xC0)
		};
		RGBC aTextColor[MULTIPAGE_NUMCOLORS]{
			/* Disabled page */	RGB_GRAYL(0x80),
			/* Enabled page */	RGB_BLACK
		};
		unsigned Align{ MULTIPAGE_ALIGN_LEFT | MULTIPAGE_ALIGN_TOP };
	} static DefaultProps;
	Properties Props;
	WM_Obj *pClient;
	GUI_ARRAY Handles;
	unsigned Selection;
	int ScrollState;

	void _AddScrollbar(int x, int y, int w, int h) {
		if (auto pScroll = (SCROLLBAR_Obj *)WM_GetScrollbarH(this)) {
			WM_MoveChildTo(pScroll, x, y);
			WM_SetSize(pScroll, w, h);
		}
		else {
			pScroll = SCROLLBAR_Create(x, y, w, h, this, GUI_ID_HSCROLL, WM_CF_SHOW, 0);
			WIDGET_SetEffect(pScroll, this->pEffect);
		}
		this->State |= MULTIPAGE_STATE_SCROLLMODE;
	}
	void _SetScrollbar(int NumItems) {
		auto pScroll = (SCROLLBAR_Obj *)WM_GetScrollbarH(this);
		pScroll->SetNumItems(NumItems);
		pScroll->SetPageSize(1);
		if (ScrollState >= NumItems)
			ScrollState = 0;
		pScroll->SetValue(this->ScrollState);
	}
	void _DeleteScrollbar() {
		WM_DeleteWindow(WM_GetScrollbarH(this));
		this->State &= ~MULTIPAGE_STATE_SCROLLMODE;
	}
	void _ShowPage(unsigned Index) {
		WM_Obj *hWin = 0;
		auto pClient = this->pClient;
		if ((int)Index < this->Handles.NumItems) {
			MULTIPAGE_PAGE *pPage;
			pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&this->Handles, Index);
			hWin = pPage->hWin;
		}
		for (auto pChild = pClient->pFirstChild; pChild; pChild = pChild->pNext) {
			if (pChild == hWin) {
				WM_ShowWindow(pChild);
				WM_SetFocus(pChild);
			}
			else
				WM_HideWindow(pChild);
		}
	}
	void _SetEnable(unsigned Index, int State) {
		if ((int)Index < this->Handles.NumItems) {
			MULTIPAGE_PAGE *pPage;
			pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&this->Handles, Index);
			if (State) {
				pPage->Status |= MULTIPAGE_STATE_ENABLED;
			}
			else {
				pPage->Status &= ~MULTIPAGE_STATE_ENABLED;
			}
		}
	}
	int _GetEnable(unsigned Index) {
		int r = 0;
		if ((int)Index < this->Handles.NumItems) {
			MULTIPAGE_PAGE *pPage;
			pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&this->Handles, Index);
			r = (pPage->Status & MULTIPAGE_STATE_ENABLED) ? 1 : 0;
		}
		return r;
	}
	void _CalcClientRect(GUI_RECT *pRect) {
		*pRect = WIDGET__GetInsideRect(this);
		if (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
			pRect->y1 -= GUI_GetYSizeOfFont(this->Props.Font) + 6;
		}
		else {
			pRect->y0 += GUI_GetYSizeOfFont(this->Props.Font) + 6;
		}
	}
	void _CalcBorderRect(GUI_RECT *pRect) {
		*pRect = WM_GetClientRect(this);
		if (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
			pRect->y1 -= GUI_GetYSizeOfFont(this->Props.Font) + 6;
		}
		else {
			pRect->y0 += GUI_GetYSizeOfFont(this->Props.Font) + 6;
		}
	}
	int _GetPageSizeX(unsigned Index) {
		int r = 0;
		if ((int)Index < this->Handles.NumItems) {
			MULTIPAGE_PAGE *pPage;
			GUI_SetFont(this->Props.Font);
			pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&this->Handles, Index);
			r = GUI_GetStringDistX(&pPage->acText) + 10;
		}
		return r;
	}
	int _GetPagePosX(unsigned Index) {
		unsigned i, r = 0;
		for (i = 0; i < Index; i++) {
			r += _GetPageSizeX(i);
		}
		return r;
	}
	int _GetTextWidth() {
		return _GetPagePosX(this->Handles.NumItems);
	}
	void _GetTextRect(GUI_RECT *pRect) {
		GUI_RECT rBorder;
		int Width, Height;
		Height = GUI_GetYSizeOfFont(this->Props.Font) + 6;
		_CalcBorderRect(&rBorder);
		/* Calculate Y-Position of text item */
		if (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
			pRect->y0 = rBorder.y1;
		}
		else {
			pRect->y0 = 0;
		}
		pRect->y1 = pRect->y0 + Height;
		/* Calculate width of text items */
		if (this->State & MULTIPAGE_STATE_SCROLLMODE) {
			Width = rBorder.x1 - ((Height * 3) >> 1) - 3;
		}
		else {
			Width = _GetTextWidth();
		}
		/* Calculate X-Position of text item */
		if (this->Props.Align & MULTIPAGE_ALIGN_RIGHT) {
			pRect->x0 = rBorder.x1 - Width;
			pRect->x1 = rBorder.x1;
		}
		else {
			pRect->x0 = 0;
			pRect->x1 = Width;
		}
	}
	void _UpdatePositions() {
		GUI_RECT rBorder;
		int Width;
		Width = _GetTextWidth();
		_CalcBorderRect(&rBorder);
		/* Set scrollmode according to the text width */
		if (Width > rBorder.x1) {
			GUI_RECT rText;
			int Size, x0, y0, NumItems = 0;
			Size = ((GUI_GetYSizeOfFont(this->Props.Font) + 6) * 3) >> 2;
			x0 = (this->Props.Align & MULTIPAGE_ALIGN_RIGHT) ? (rBorder.x0) : (rBorder.x1 - 2 * Size + 1);
			y0 = (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) ? (rBorder.y1) : (rBorder.y0 - Size + 1);
			/* A scrollbar is required so we add one to the multipage */
			_AddScrollbar(x0, y0, 2 * Size, Size);
			_GetTextRect(&rText);
			while (Width >= Max((rText.x1 - rText.x0 + 1), 1)) {
				Width -= _GetPageSizeX(NumItems++);
			}
			_SetScrollbar(NumItems + 1);
		}
		else {
			/* Scrollbar is no longer required. We delete it if there was one */
			_DeleteScrollbar();
		}
		/* Move and resize the client area to the updated positions */
		_CalcClientRect(&rBorder);
		WM_MoveChildTo(this->pClient, rBorder.x0, rBorder.y0);
		WM_SetSize(this->pClient, rBorder.x1 - rBorder.x0 + 1, rBorder.y1 - rBorder.y0 + 1);
		WM_Invalidate(this);
	}
	void _DrawTextItem(const char *pText, unsigned Index,
					   const GUI_RECT *pRect, int x0, int w, int ColorIndex) {
		GUI_RECT r;
		r = *pRect;
		r.x0 += x0;
		r.x1 = r.x0 + w;
		WIDGET__EFFECT_DrawUpRect(this, r);
		r -= this->pEffect->EffectSize;
		if (this->Selection == Index) {
			if (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
				r.y0 -= this->pEffect->EffectSize + 1;
				if (this->pEffect->EffectSize > 1) {
					GUI_SetColor(RGB_WHITE);
					GUI_DrawVLine(r.x0 - 1, r.y0, r.y0 + 1);
					GUI_SetColor(RGB_GRAYL(0x55));
					GUI_DrawVLine(r.x1 + 1, r.y0, r.y0 + 1);
				}
			}
			else {
				r.y1 += this->pEffect->EffectSize + 1;
				if (this->pEffect->EffectSize > 1) {
					GUI_SetColor(RGB_WHITE);
					GUI_DrawVLine(r.x0 - 1, r.y1 - 2, r.y1 - 1);
					GUI_SetColor(RGB_GRAYL(0x55));
					GUI_DrawVLine(r.x1 + 1, r.y1 - 2, r.y1 - 1);
				}
			}
		}
		GUI_SetColor(this->Props.aBkColor[ColorIndex]);
		WIDGET__FillRect(this, r);
		GUI_SetBkColor(this->Props.aBkColor[ColorIndex]);
		GUI_SetColor(this->Props.aTextColor[ColorIndex]);
		GUI_DispStringAt(pText, r.x0 + 4, pRect->y0 + 3);
	}
	void _OnPaint() {
		GUI_RECT rBorder;
		/* Draw border of multipage */
		_CalcBorderRect(&rBorder);
		WIDGET__EFFECT_DrawUpRect(this, rBorder);
		/* Draw text items */
		if (this->Handles.NumItems > 0) {
			MULTIPAGE_PAGE *pPage;
			GUI_RECT rText, rClip;
			int i, w = 0, x0 = 0;
			if (this->State & MULTIPAGE_STATE_SCROLLMODE) {
				if (this->Props.Align & MULTIPAGE_ALIGN_RIGHT) {
					x0 = -_GetPagePosX(this->ScrollState);
				}
				else {
					x0 = -_GetPagePosX(this->ScrollState);
				}
			}
			_GetTextRect(&rText);
			rClip = rText;
			rClip.y0 = rText.y0 - 1;
			rClip.y1 = rText.y1 + 1;
			WM_SetUserClipRect(&rClip);
			GUI_SetFont(this->Props.Font);
			for (i = 0; i < this->Handles.NumItems; i++) {
				pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&this->Handles, i);
				x0 += w;
				w = GUI_GetStringDistX(&pPage->acText) + 10;
				_DrawTextItem(&pPage->acText, i, &rText, x0, w, (pPage->Status & MULTIPAGE_STATE_ENABLED) ? 1 : 0);
			}
			WM_SetUserClipRect(nullptr);
		}
	}
	int _ClickedOnMultipage(int x, int y) {
		GUI_RECT rText;
		_GetTextRect(&rText);
		if ((y >= rText.y0) && (y <= rText.y1)) {
			if ((this->Handles.NumItems > 0) && (x >= rText.x0) && (x <= rText.x1)) {
				int i, w = 0, x0 = rText.x0;
				/* Check if another page must be selected */
				if (this->State & MULTIPAGE_STATE_SCROLLMODE) {
					x0 -= _GetPagePosX(this->ScrollState);
				}
				for (i = 0; i < this->Handles.NumItems; i++) {
					x0 += w;
					w = _GetPageSizeX(i);
					if (x >= x0 && x <= (x0 + w - 1)) {
						SelectPage(i);
						WM_NotifyParent(this, WM_NOTIFICATION_VALUE_CHANGED);
						return 1;
					}
				}
			}
			return 0;
		}
		return 1;
	}
	void _OnTouch(const GUI_PID_STATE *pState) {
		int Notification;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				int x = pState->x;
				int y = pState->y;
				if (!_ClickedOnMultipage(x, y)) {
					WM_Obj *hBelow;
					x += WM_GetWindowOrgX(this);
					y += WM_GetWindowOrgY(this);
					hBelow = WM_Screen2hWinEx(this, x, y);
					if (hBelow) {
						GUI_PID_STATE State;
						State.x = x - WM_GetWindowOrgX(hBelow);
						State.y = y - WM_GetWindowOrgY(hBelow);
						State.Pressed = pState->Pressed;
						((WM_Obj *)hBelow)->cb(hBelow, WM_TOUCH, (WM_PARAM)&State);
					}
				}
				else
					WM_BringToTop(this);
				Notification = WM_NOTIFICATION_CLICKED;
			}
			else
				Notification = WM_NOTIFICATION_RELEASED;
		}
		else
			Notification = WM_NOTIFICATION_MOVED_OUT;
		WM_NotifyParent(this, Notification);
	}

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (MULTIPAGE_Obj *)hWin;
		auto Handled = WIDGET_HandleActive(pObj, MsgId, &Data);
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const GUI_PID_STATE *)Data);
				return 0;
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const WM_NOTIFY_INFO *)Data;
				auto pWinSrc = pInfo->pWinSrc;
				if (pInfo->Notification == WM_NOTIFICATION_VALUE_CHANGED) {
					if (WM_GetId(pWinSrc) == GUI_ID_HSCROLL) {
						pObj->ScrollState = ((SCROLLBAR_Obj *)pWinSrc)->GetValue();
						WM_Invalidate(pObj);
					}
				}
				return 0;
			}
			case WM_GET_CLIENT_WINDOW:
				return (WM_PARAM)pObj->pClient;
			case WM_GET_INSIDE_RECT:
				pObj->_CalcClientRect((GUI_RECT *)(Data));
				return 0;
			case WM_WIDGET_SET_EFFECT:
				WIDGET_SetEffect(WM_GetScrollbarH(pObj), (WIDGET_EFFECT const *)Data);
			case WM_SIZE:
				pObj->_UpdatePositions();
				return 0;
			case WM_DELETE:
				GUI_ARRAY_Delete(&pObj->Handles);
				/* No break here ... WM_DefaultProc needs to be called */
			default:
				/* Let widget handle the standard messages */
				if (!Handled)
					return Data;
				return WM_DefaultProc(hWin, MsgId, Data);
		}
		return 0;
	}
	static WM_PARAM _ClientCallback(WM_Obj *pObj, int MsgId, WM_PARAM Data) {
		auto pParent = (MULTIPAGE_Obj *)WM_GetParent(pObj);
		switch (MsgId) {
			case WM_PAINT:
				GUI_SetBkColor(pParent->Props.aBkColor[1]);
				GUI_Clear();
				return 0;
			case WM_TOUCH:
				WM_SetFocus(pParent);
				WM_BringToTop(pParent);
				return 0;
			case WM_GET_CLIENT_WINDOW:
				return (WM_PARAM)pObj;
			case WM_GET_INSIDE_RECT:
				return WM_DefaultProc(pObj, MsgId, Data);
		}
		return 0;
	}

public:
	void AddPage(WM_Obj *hWin, const char *pText) {
		GUI_USE_PARA(hWin);
		if (!hWin) {
			/* If we get no handle we must find it. To do this, we search      */
			/* all children until we found one that has not yet become a page. */
			auto pClient = this->pClient;
			for (auto pChild = pClient->pFirstChild; pChild && !hWin; pChild = pChild->pNext) {
				hWin = pChild;
				for (int i = 0; i < Handles.NumItems; i++) {
					auto pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&Handles, i);
					if (pPage->hWin == pChild) {
						hWin = 0;
						break;
					}
				}
			}
		}
		else {
			/* If we get a handle we must ensure that it was attached to the multipage */
			WM_AttachWindowAt(hWin, this->pClient, 0, 0);
		}
		if (hWin) {
			MULTIPAGE_PAGE Page;
			char NullByte = 0;
			if (!pText) {
				pText = &NullByte;
			}
			Page.hWin = hWin;
			Page.Status = MULTIPAGE_STATE_ENABLED;
			if (GUI_ARRAY_AddItem(&Handles, &Page, sizeof(MULTIPAGE_PAGE) + GUI__strlen(pText)) == 0) {
				MULTIPAGE_PAGE *pPage;
				pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&Handles, Handles.NumItems - 1);
				GUI__memcpy(&pPage->acText, pText, GUI__strlen(pText) + 1);
			}
			SelectPage(Handles.NumItems - 1);
		}
	}
	void DeletePage(unsigned Index, int Delete) {
		if ((int)Index < Handles.NumItems) {
			WM_Obj *hWin;
			MULTIPAGE_PAGE *pPage;
			pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&Handles, Index);
			hWin = pPage->hWin;
			/* Remove the page from the multipage object */
			if (Index == this->Selection) {
				if (Index == ((unsigned)Handles.NumItems - 1)) {
					this->_ShowPage(Index - 1);
					this->Selection--;
				}
				else {
					this->_ShowPage(Index + 1);
				}
			}
			else {
				if (Index < this->Selection) {
					this->Selection--;
				}
			}
			GUI_ARRAY_DeleteItem(&Handles, Index);
			this->_UpdatePositions();
			/* Delete the window of the page */
			if (Delete) {
				WM_DeleteWindow(hWin);
			}
		}
	}
	void SelectPage(unsigned Index) {
		if ((int)Index < Handles.NumItems) {
			if (Index != this->Selection && this->_GetEnable(Index)) {
				this->_ShowPage(Index);
				this->Selection = Index;
				this->_UpdatePositions();
			}
		}
	}
	void DisablePage(unsigned Index) {
		this->_SetEnable(Index, 0);
		WM_Invalidate(this);
	}
	void EnablePage(unsigned Index) {
		this->_SetEnable(Index, 1);
		WM_Invalidate(this);
	}
	void SetText(const char *pText, unsigned Index) {
		if (pText) {
			if ((int)Index < Handles.NumItems) {
				MULTIPAGE_PAGE *pPage;
				MULTIPAGE_PAGE Page;
				pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&Handles, Index);
				Page.hWin = pPage->hWin;
				Page.Status = pPage->Status;
				if (GUI_ARRAY_SetItem(&Handles, Index, &Page, sizeof(MULTIPAGE_PAGE) + GUI__strlen(pText))) {
					pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&Handles, Index);
					GUI__memcpy(&pPage->acText, pText, GUI__strlen(pText) + 1);
					this->_UpdatePositions();
				}
			}
		}
	}
	void SetBkColor(RGBC Color, unsigned Index) {
		if (((int)Index < MULTIPAGE_NUMCOLORS)) {
			this->Props.aBkColor[Index] = Color;
			WM_Invalidate(this);
		}
	}
	void SetTextColor(RGBC Color, unsigned Index) {
		if (((int)Index < MULTIPAGE_NUMCOLORS)) {
			this->Props.aTextColor[Index] = Color;
			WM_Invalidate(this);
		}
	}
	void SetFont(PCFONT pFont) {
		if (pFont) {
			this->Props.Font = pFont;
			this->_UpdatePositions();
		}
	}
	void SetAlign(unsigned Align) {
		GUI_RECT rClient;
		this->Props.Align = Align;
		this->_CalcClientRect(&rClient);
		WM_MoveTo(this->pClient, rClient.x0 + this->Rect.x0,
				  rClient.y0 + this->Rect.y0);
		this->_UpdatePositions();
	}
	int  GetSelection() {
		return Selection;
	}
	WM_Obj *GetWindow(unsigned Index) {
		WM_Obj *r = 0;
		if ((int)Index < Handles.NumItems) {
			MULTIPAGE_PAGE *pPage;
			pPage = (MULTIPAGE_PAGE *)GUI_ARRAY_GetpItem(&Handles, Index);
			r = pPage->hWin;
		}

		return r;
	}
	int IsPageEnabled(unsigned Index) {
		int r = 0;
		r = this->_GetEnable(Index);

		return r;
	}

};

MULTIPAGE_Obj::Properties MULTIPAGE_Obj::DefaultProps;

MULTIPAGE_Obj *MULTIPAGE_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
								  int WinFlags, int ExFlags, int Id) {
	/* Create the window */
	auto pObj = (MULTIPAGE_Obj *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags | WM_CF_HASTRANS, MULTIPAGE_Obj::_Callback,
								  sizeof(MULTIPAGE_Obj) - sizeof(WM_Obj));
	if (pObj) {
		GUI_RECT rClient;
		int Flags;
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = MULTIPAGE_Obj::DefaultProps;
		pObj->Selection = 0xffff;
		pObj->ScrollState = 0;
		pObj->State = 0;
		pObj->_CalcClientRect(&rClient);
		Flags = WM_CF_SHOW | WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_RIGHT | WM_CF_ANCHOR_TOP | WM_CF_ANCHOR_BOTTOM;
		pObj->pClient = (WM_Obj *)WM_CreateWindowAsChild(rClient.x0, rClient.y0,
														 rClient.x1 - rClient.x0 + 1,
														 rClient.y1 - rClient.y0 + 1,
														 pObj, Flags, MULTIPAGE_Obj::_ClientCallback, 0);
		pObj->_UpdatePositions();
	}
	else {
	}
	return pObj;
}
MULTIPAGE_Obj *MULTIPAGE_Create(int x0, int y0, int xsize, int ysize,
								WM_Obj *hParent, int Id, int Flags, int ExFlags) {
	return MULTIPAGE_CreateEx(x0, y0, xsize, ysize, hParent, Flags, ExFlags, Id);
}
WM_Obj *MULTIPAGE_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo,
								 WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	return MULTIPAGE_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							   hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
}

}
