module;

#include "DIALOG_Intern.h"

export module TUX.Widget.MultPage;

import TUX.Widget;
import TUX.Widget.ScrollBar;

import TUX.Array;

	constexpr uint16_t MULTIPAGE_STATE_ENABLED     = (1<<0);
constexpr uint16_t MULTIPAGE_STATE_SCROLLMODE  = WIDGET_STATE_USER<0>;
constexpr uint16_t MULTIPAGE_NUMCOLORS         = 2;

export {
constexpr uint16_t MULTIPAGE_ALIGN_LEFT   = 0 << 0;
constexpr uint16_t MULTIPAGE_ALIGN_RIGHT  = 1 << 0;
constexpr uint16_t MULTIPAGE_ALIGN_TOP    = 0 << 2;
constexpr uint16_t MULTIPAGE_ALIGN_BOTTOM = 1 << 2;

struct MULTIPAGE_PAGE {
	WObj *hWin;
	uint8_t Status;
	char *pText;
};

class MultPage : public WIDGET {

public:
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
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
	
private:
	Properties Props;
	
	WObj *pClient;
	ARRAY<MULTIPAGE_PAGE> Handles;
	unsigned Selection;
	int ScrollState;

	void _AddScrollbar(int x, int y, int w, int h) {
		if (auto pScroll = GetScrollbarH()) {
			WM_MoveChildTo(pScroll, x, y);
			WM_SetSize(pScroll, w, h);
		}
		else {
			auto pScrollbar = ScrollBar::Create(x, y, w, h, this, GUI_ID_HSCROLL, WC_VISIBLE, 0);
			pScrollbar->SetEffect(this->pEffect);
		}
		this->State |= MULTIPAGE_STATE_SCROLLMODE;
	}
	void _SetScrollbar(int NumItems) {
		auto pScroll = (ScrollBar *)GetScrollbarH();
		pScroll->SetNumItems(NumItems);
		pScroll->SetPageSize(1);
		if (ScrollState >= NumItems)
			ScrollState = 0;
		pScroll->SetValue(this->ScrollState);
	}
	void _DeleteScrollbar() {
		WM_DeleteWindow(GetScrollbarH());
		this->State &= ~MULTIPAGE_STATE_SCROLLMODE;
	}
	void _ShowPage(unsigned Index) {
		WObj *hWin = 0;
		auto pClient = this->pClient;
		if ((int)Index < this->Handles.NumItems) {
			hWin = this->Handles[Index].hWin;
		}
		for (auto pChild = pClient->pFirstChild; pChild; pChild = pChild->pNext) {
			if (pChild == hWin) {
				pChild->ShowWindow();
				WM_SetFocus(pChild);
			}
			else
				pChild->HideWindow();
		}
	}
	void _SetEnable(unsigned Index, int State) {
		if ((int)Index < this->Handles.NumItems) {
			auto &pPage = this->Handles[Index];
			if (State) {
				pPage.Status |= MULTIPAGE_STATE_ENABLED;
			}
			else {
				pPage.Status &= ~MULTIPAGE_STATE_ENABLED;
			}
		}
	}
	int _GetEnable(unsigned Index) {
		int r = 0;
		if ((int)Index < this->Handles.NumItems) {
			r = (this->Handles[Index].Status & MULTIPAGE_STATE_ENABLED) ? 1 : 0;
		}
		return r;
	}
	void _CalcClientRect(RECT *pRect) {
		*pRect = WIDGET__GetInsideRect(this);
		if (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
			pRect->y1 -= Props.pFont->SizeY() + 6;
		}
		else {
			pRect->y0 += Props.pFont->SizeY() + 6;
		}
	}
	void _CalcBorderRect(RECT *pRect) {
		*pRect = WM_GetClientRect(this);
		if (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
			pRect->y1 -= Props.pFont->SizeY() + 6;
		}
		else {
			pRect->y0 += Props.pFont->SizeY() + 6;
		}
	}
	int _GetPageSizeX(unsigned Index) {
		int r = 0;
		if ((int)Index < this->Handles.NumItems) {
			GUI_SetFont(this->Props.pFont);
			r = GUI_GetStringDistX(this->Handles[Index].pText) + 10;
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
	void _GetTextRect(RECT *pRect) {
		RECT rBorder;
		int Width, Height;
		Height = Props.pFont->SizeY() + 6;
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
		RECT rBorder;
		int Width;
		Width = _GetTextWidth();
		_CalcBorderRect(&rBorder);
		/* Set scrollmode according to the text width */
		if (Width > rBorder.x1) {
			RECT rText;
			int x0, y0, NumItems = 0;
			auto Size = ((Props.pFont->SizeY() + 6) * 3) >> 2;
			x0 = (this->Props.Align & MULTIPAGE_ALIGN_RIGHT) ? (rBorder.x0) : (rBorder.x1 - 2 * Size + 1);
			y0 = (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) ? (rBorder.y1) : (rBorder.y0 - Size + 1);
			/* A scrollbar is required so we add one to the MultPage */
			_AddScrollbar(x0, y0, 2 * Size, Size);
			_GetTextRect(&rText);
			while (Width >= Max((rText.x1 - rText.x0 + 1), 1)) {
				Width -= _GetPageSizeX(NumItems++);
			}
			_SetScrollbar(NumItems + 1);
		}
		else {
			/* ScrollBar is no longer required. We delete it if there was one */
			_DeleteScrollbar();
		}
		/* Move and resize the client area to the updated positions */
		_CalcClientRect(&rBorder);
		WM_MoveChildTo(this->pClient, rBorder.x0, rBorder.y0);
		WM_SetSize(this->pClient, rBorder.x1 - rBorder.x0 + 1, rBorder.y1 - rBorder.y0 + 1);
		WM_Invalidate(this);
	}
	void _DrawTextItem(const char *pText, unsigned Index,
					   const RECT *pRect, int x0, int w, int ColorIndex) {
		RECT r;
		r = *pRect;
		r.x0 += x0;
		r.x1 = r.x0 + w;
		DrawUp(r);
		r -= this->EffectSize();
		if (this->Selection == Index) {
			if (this->Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
				r.y0 -= this->EffectSize() + 1;
				if (this->EffectSize() > 1) {
					GUI.SetColor(RGB_WHITE);
					GUI_DrawVLine(r.x0 - 1, r.y0, r.y0 + 1);
					GUI.SetColor(RGB_GRAYL(0x55));
					GUI_DrawVLine(r.x1 + 1, r.y0, r.y0 + 1);
				}
			}
			else {
				r.y1 += this->EffectSize() + 1;
				if (this->EffectSize() > 1) {
					GUI.SetColor(RGB_WHITE);
					GUI_DrawVLine(r.x0 - 1, r.y1 - 2, r.y1 - 1);
					GUI.SetColor(RGB_GRAYL(0x55));
					GUI_DrawVLine(r.x1 + 1, r.y1 - 2, r.y1 - 1);
				}
			}
		}
		GUI.SetColor(this->Props.aBkColor[ColorIndex]);
		GUI_FillRect(r);
		GUI.SetBkColor(this->Props.aBkColor[ColorIndex]);
		GUI.SetColor(this->Props.aTextColor[ColorIndex]);
		GUI_DispStringAt(pText, r.x0 + 4, pRect->y0 + 3);
	}
	void _OnPaint() {
		RECT rBorder;
		/* Draw border of MultPage */
		_CalcBorderRect(&rBorder);
		DrawUp(rBorder);
		/* Draw text items */
		if (this->Handles.NumItems > 0) {
			RECT rText, rClip;
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
			GUI_SetFont(this->Props.pFont);
			for (i = 0; i < this->Handles.NumItems; i++) {
				auto &pPage = this->Handles[i];
				x0 += w;
				w = GUI_GetStringDistX(pPage.pText) + 10;
				_DrawTextItem(pPage.pText, i, &rText, x0, w, (pPage.Status & MULTIPAGE_STATE_ENABLED) ? 1 : 0);
			}
			WM_SetUserClipRect(nullptr);
		}
	}
	int _ClickedOnMultipage(int x, int y) {
		RECT rText;
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
	void _OnTouch(const PID_STATE *pState) {
		int Notification;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				POINT Pos = *pState;
				if (!_ClickedOnMultipage(Pos.x, Pos.y)) {
					Pos += GetOrg();
					if (auto pBelow = WM_Screen2hWinEx(this, Pos.x, Pos.y)) {
						PID_STATE State;
						State = Pos - pBelow->GetOrg();
						State.Pressed = pState->Pressed;
						pBelow->cb(pBelow, WM_TOUCH, (WM_PARAM)&State);
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

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (MultPage *)hWin;
		auto Handled = WIDGET_HandleActive(pObj, MsgId, &Data);
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const NOTIFY_INFO *)Data;
				auto pWinSrc = pInfo->pWinSrc;
				if (pInfo->Notification == WM_NOTIFICATION_VALUE_CHANGED) {
					if (pWinSrc->GetID() == GUI_ID_HSCROLL) {
						pObj->ScrollState = ((ScrollBar *)pWinSrc)->GetValue();
						WM_Invalidate(pObj);
					}
				}
				return 0;
			}
			case WM_GET_CLIENT_WINDOW:
				return (WM_PARAM)pObj->pClient;
			case WM_GET_INSIDE_RECT:
				pObj->_CalcClientRect((RECT *)Data);
				return 0;
			case WM_WIDGET_SET_EFFECT:
				if (auto pScroll = (ScrollBar *)pObj->GetScrollbarH())
					pScroll->SetEffect((const WIDGET_EFFECT *)Data);
			case WM_SIZE:
				pObj->_UpdatePositions();
				return 0;
			case WM_DELETE: {
				for (int _i = 0; _i < pObj->Handles.NumItems; _i++) {
					GUI_ALLOC_FreePtr((void **)&pObj->Handles[_i].pText);
				}
				pObj->Handles.Delete();
				/* No break here ... WM_DefaultProc needs to be called */
			}
			default:
				/* Let widget handle the standard messages */
				if (!Handled)
					return Data;
				return WM_DefaultProc(hWin, MsgId, Data);
		}
		return 0;
	}
	static WM_PARAM _ClientCallback(WObj *pObj, int MsgId, WM_PARAM Data) {
		auto pParent = (MultPage *)pObj->Parent();
		switch (MsgId) {
			case WM_PAINT:
				GUI.SetBkColor(pParent->Props.aBkColor[1]);
				GUI_Clear();
				return 0;
			case WM_GET_BKCOLOR:
				return (WM_PARAM)pParent->Props.aBkColor[1];
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

	static MultPage *Create(int x0, int y0, int xsize, int ysize, WObj *hParent,
							int WinFlags, int ExFlags, int Id) {
		/* Create the window */
		auto pObj = (MultPage *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags | WC_HASTRANS, MultPage::_Callback,
													   sizeof(MultPage) - sizeof(WObj));
		if (!pObj) {
			GUI_DEBUG_ERROROUT_IF(pObj == 0, "MultPage create failed");
			return nullptr;
		}
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* init member variables */
		pObj->Props = MultPage::DefaultProps;
		pObj->Selection = 0xffff;
		pObj->ScrollState = 0;
		pObj->State = 0;
		RECT rClient;
		pObj->_CalcClientRect(&rClient);
		pObj->pClient = (WObj *)WM_CreateWindowAsChild(
			rClient.x0, rClient.y0,
			rClient.x1 - rClient.x0 + 1,
			rClient.y1 - rClient.y0 + 1,
			pObj, WC_VISIBLE | WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT | WC_ANCHOR_TOP | WC_ANCHOR_BOTTOM, MultPage::_ClientCallback, 0);
		pObj->_UpdatePositions();
		return pObj;
	}
	static WObj *CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo,
								WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return Create(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
					  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
	}

public:

	void AddPage(WObj *hWin, const char *pText) {
		GUI_USE_PARA(hWin);
		if (!hWin) {
			/* If we get no handle we must find it. To do this, we search      */
			/* all children until we found one that has not yet become a page. */
			auto pClient = this->pClient;
			for (auto pChild = pClient->pFirstChild; pChild && !hWin; pChild = pChild->pNext) {
				hWin = pChild;
				for (int i = 0; i < Handles.NumItems; i++) {
					auto &pPage = Handles[i];
						if (pPage.hWin == pChild) {
						hWin = 0;
						break;
					}
				}
			}
		}
		else {
			/* If we get a handle we must ensure that it was attached to the MultPage */
			WM_AttachWindowAt(hWin, this->pClient, 0, 0);
		}
		if (hWin) {
			MULTIPAGE_PAGE Page = {};
			char NullByte = 0;
			if (!pText) {
				pText = &NullByte;
			}
			Page.hWin = hWin;
			Page.Status = MULTIPAGE_STATE_ENABLED;
			if (Handles.AddItem(&Page) == 0) {
				GUI__SetText(&Handles[Handles.NumItems - 1].pText, pText);
			}
			SelectPage(Handles.NumItems - 1);
		}
	}
	void DeletePage(unsigned Index, int Delete) {
		if ((int)Index < Handles.NumItems) {
			WObj *hWin;
			hWin = Handles[Index].hWin;
			/* Remove the page from the MultPage object */
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
			GUI_ALLOC_FreePtr((void **)&Handles[Index].pText);
			Handles.DeleteItem(Index);
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
		if (pText && (int)Index < Handles.NumItems) {
			if (GUI__SetText(&Handles[Index].pText, pText))
				this->_UpdatePositions();
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
			this->Props.pFont = pFont;
			this->_UpdatePositions();
		}
	}
	void SetAlign(unsigned Align) {
		RECT rClient;
		this->Props.Align = Align;
		this->_CalcClientRect(&rClient);
		WM_MoveTo(this->pClient, rClient.x0 + this->Rect.x0,
				  rClient.y0 + this->Rect.y0);
		this->_UpdatePositions();
	}
	int  GetSelection() {
		return Selection;
	}
	WObj *GetWindow(unsigned Index) {
		WObj *r = 0;
		if ((int)Index < Handles.NumItems) {
			r = Handles[Index].hWin;
		}

		return r;
	}
	int IsPageEnabled(unsigned Index) {
		int r = 0;
		r = this->_GetEnable(Index);

		return r;
	}

};

MultPage::Properties MultPage::DefaultProps;

}
