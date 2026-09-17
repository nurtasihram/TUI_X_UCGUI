module;

#include "GUI.h"

export module TUX.Widget.MultPage;

import TUX.Widget;
import TUX.Widget.ScrollBar;

import TUX.Array;

constexpr uint16_t MULTIPAGE_STATE_ENABLED     = 1 << 0;

constexpr uint16_t MULTIPAGE_STATE_SCROLLMODE  = WIDGET_STATE_USER<0>;

constexpr uint16_t MULTIPAGE_NUMCOLORS         = 2;

export {

constexpr uint16_t
	MULTIPAGE_ALIGN_LEFT   = 0 << 0,
	MULTIPAGE_ALIGN_RIGHT  = 1 << 0,
	MULTIPAGE_ALIGN_TOP    = 0 << 2,
	MULTIPAGE_ALIGN_BOTTOM = 1 << 2;

class MultPage : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		RGBC aBkColor[MULTIPAGE_NUMCOLORS]{
			/* Disabled page */	RGBC::Gray(0xD0),
			/* Enabled page */	RGBC::Gray(0xC0)
		};
		RGBC aTextColor[MULTIPAGE_NUMCOLORS]{
			/* Disabled page */	RGBC::Gray(0x80),
			/* Enabled page */	RGB_BLACK
		};
		unsigned Align{ MULTIPAGE_ALIGN_LEFT | MULTIPAGE_ALIGN_TOP };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;

	struct Page {
		WObj *pWin;
		uint8_t Status;
		char *pText;
	};
	ARRAY<Page> Handles;
	WObj *pClient;
	uint16_t sel = 0xffff;
	int16_t ScrollState = 0;

	void _AddScrollbar(int x, int y, int w, int h) {
		if (auto pScroll = GetScrollbarH()) {
			pScroll->MoveChildTo({ x, y });
			pScroll->Size({ w, h });
		}
		else {
			auto pScrollbar = new ScrollBar(
				RECT::LeftTop({ x, y }, { w, h }),
				WC_VISIBLE, this, GUI_ID_HSCROLL,
				0);
			pScrollbar->SetEffect(GetEffect());
		}
		AddStates(MULTIPAGE_STATE_SCROLLMODE);
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
		delete GetScrollbarH();
		DelStates(MULTIPAGE_STATE_SCROLLMODE);
	}

	void _ShowPage(uint16_t Index) {
		WObj *pWin = nullptr;
		if (Index < Handles.NumItems())
			pWin = Handles[Index].pWin;
		for (auto pChild = pClient->FirstChild(); pChild; pChild = pChild->NextSibling()) {
			if (pChild == pWin) {
				pChild->ShowWindow();
				pChild->SetFocus();
			}
			else
				pChild->HideWindow();
		}
	}
	void _SetEnable(uint16_t Index, int bEnable) {
		if (Index < Handles.NumItems()) {
			auto &page = Handles[Index];
			if (bEnable)
				page.Status |= MULTIPAGE_STATE_ENABLED;
			else
				page.Status &= ~MULTIPAGE_STATE_ENABLED;
		}
	}
	
	void _CalcClientRect(RECT *pRect) {
		*pRect = _GetInsideRect();
		if (Props.Align & MULTIPAGE_ALIGN_BOTTOM)
			pRect->y1 -= Props.pFont->YSize + 6;
		else
			pRect->y0 += Props.pFont->YSize + 6;
	}
	void _CalcBorderRect(RECT *pRect) {
		*pRect = ClientRect();
		if (Props.Align & MULTIPAGE_ALIGN_BOTTOM)
			pRect->y1 -= Props.pFont->YSize + 6;
		else
			pRect->y0 += Props.pFont->YSize + 6;
	}
	uint16_t _GetPageSizeX(uint16_t Index) {
		if (Index < Handles.NumItems())
			return Props.pFont->TextBound(Handles[Index].pText).x + 10;
		return 0;
	}
	uint16_t _GetPagePosX(uint16_t Index) {
		uint16_t r = 0;
		for (uint16_t i = 0; i < Index; i++)
			r += _GetPageSizeX(i);
		return r;
	}
	int _GetTextWidth() {
		return _GetPagePosX(Handles.NumItems());
	}
	void _GetTextRect(RECT *pRect) {
		RECT rBorder;
		int Width, Height;
		Height = Props.pFont->YSize + 6;
		_CalcBorderRect(&rBorder);
		/* Calculate Y-Position of text item */
		if (Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
			pRect->y0 = rBorder.y1;
		}
		else {
			pRect->y0 = 0;
		}
		pRect->y1 = pRect->y0 + Height;
		/* Calculate width of text items */
		if (States & MULTIPAGE_STATE_SCROLLMODE) {
			Width = rBorder.x1 - ((Height * 3) >> 1) - 3;
		}
		else {
			Width = _GetTextWidth();
		}
		/* Calculate X-Position of text item */
		if (Props.Align & MULTIPAGE_ALIGN_RIGHT) {
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
			auto Size = ((Props.pFont->YSize + 6) * 3) >> 2;
			x0 = (Props.Align & MULTIPAGE_ALIGN_RIGHT) ? (rBorder.x0) : (rBorder.x1 - 2 * Size + 1);
			y0 = (Props.Align & MULTIPAGE_ALIGN_BOTTOM) ? (rBorder.y1) : (rBorder.y0 - Size + 1);
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
		pClient->MoveChildTo(rBorder.LeftTop());
		pClient->Size(rBorder.Size());
		Invalidate();
	}

	void _DrawTextItem(const char *pText, uint16_t Index,
					   const RECT *pRect, int x0, int w, int ColorIndex) {
		RECT r;
		r = *pRect;
		r.x0 += x0;
		r.x1 = r.x0 + w;
		DrawUp(r);
		r -= EffectSize();
		if (sel == Index) {
			if (Props.Align & MULTIPAGE_ALIGN_BOTTOM) {
				r.y0 -= EffectSize() + 1;
				if (EffectSize() > 1) {
					GUI.Color(RGB_WHITE);
					GUI_DrawVLine(r.x0 - 1, r.y0, r.y0 + 1);
					GUI.Color(RGBC::Gray(0x55));
					GUI_DrawVLine(r.x1 + 1, r.y0, r.y0 + 1);
				}
			}
			else {
				r.y1 += EffectSize() + 1;
				if (EffectSize() > 1) {
					GUI.Color(RGB_WHITE);
					GUI_DrawVLine(r.x0 - 1, r.y1 - 2, r.y1 - 1);
					GUI.Color(RGBC::Gray(0x55));
					GUI_DrawVLine(r.x1 + 1, r.y1 - 2, r.y1 - 1);
				}
			}
		}
		GUI.Color(Props.aBkColor[ColorIndex]);
		GUI_FillRect(r);
		GUI.BkColor(Props.aBkColor[ColorIndex]);
		GUI.Color(Props.aTextColor[ColorIndex]);
		GUI_DispStringAt(pText, r.x0 + 4, pRect->y0 + 3);
	}
	void _OnPaint() {
		SetBkColorPrefer(RGB_INVALID);
		GUI_Clear();
		RECT rBorder;
		/* Draw border of MultPage */
		_CalcBorderRect(&rBorder);
		DrawUp(rBorder);
		/* Draw text items */
		auto NumItems = Handles.NumItems();
		if (!NumItems)
			return;
		int w = 0, x0 = 0;
		if (States & MULTIPAGE_STATE_SCROLLMODE) {
			if (Props.Align & MULTIPAGE_ALIGN_RIGHT)
				x0 = -_GetPagePosX(ScrollState);
			else
				x0 = -_GetPagePosX(ScrollState);
		}
		RECT rText, rClip;
		_GetTextRect(&rText);
		rClip = rText;
		rClip.y0 = rText.y0 - 1;
		rClip.y1 = rText.y1 + 1;
		SetUserClipRect(&rClip);
		GUI.Font(Props.pFont);
		for (int i = 0; i < NumItems; i++) {
			auto &page = Handles[i];
			x0 += w;
			w = Props.pFont->TextBound(page.pText).x + 10;
			_DrawTextItem(page.pText, i, &rText, x0, w, (page.Status & MULTIPAGE_STATE_ENABLED) ? 1 : 0);
		}
		SetUserClipRect(nullptr);
	}
	int _ClickedOnMultipage(int x, int y) {
		RECT rText;
		_GetTextRect(&rText);
		if ((y >= rText.y0) && (y <= rText.y1)) {
			if ((Handles.NumItems() > 0) && (x >= rText.x0) && (x <= rText.x1)) {
				int i, w = 0, x0 = rText.x0;
				/* Check if another page must be selected */
				if (States & MULTIPAGE_STATE_SCROLLMODE) {
					x0 -= _GetPagePosX(this->ScrollState);
				}
				for (i = 0; i < Handles.NumItems(); i++) {
					x0 += w;
					w = _GetPageSizeX(i);
					if (x >= x0 && x <= (x0 + w - 1)) {
						Selected(i);
						NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
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
					Pos += LeftTop();
					if (auto pBelow = WM_Screen2Win(Pos, this)) {
						PID_STATE State{ Pos - pBelow->LeftTop(), pState->Pressed };
						pBelow->Require(WM_TOUCH, (WM_PARAM)&State);
					}
				}
				else
					BringToTop();
				Notification = WM_NOTIFICATION_CLICKED;
			}
			else
				Notification = WM_NOTIFICATION_RELEASED;
		}
		else
			Notification = WM_NOTIFICATION_MOVED_OUT;
		NotifyParent(Notification);
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (MultPage *)pWin;
		auto Handled = pObj->HandleActive(MsgId, &Data);
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
						pObj->Invalidate();
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
				for (int _i = 0; _i < pObj->Handles.NumItems(); _i++) {
					GUI_ALLOC_FreePtr((void **)&pObj->Handles[_i].pText);
				}
				pObj->Handles.Delete();
				/* No break here ... DefaultProc needs to be called */
			}
			default:
				/* Let widget handle the standard messages */
				if (!Handled)
					return Data;
				return DefaultProc(pWin, MsgId, Data);
		}
		return 0;
	}
	static WM_PARAM _ClientCallback(WObj *pObj, int MsgId, WM_PARAM Data) {
		auto pParent = (MultPage *)pObj->Parent();
		switch (MsgId) {
			case WM_PAINT:
				GUI.BkColor(pParent->Props.aBkColor[1]);
				GUI_Clear();
				return 0;
			case WM_GET_BKCOLOR:
				return (WM_PARAM)pParent->Props.aBkColor[1];
			case WM_TOUCH:
				pParent->SetFocus();
				pParent->BringToTop();
				return 0;
			case WM_GET_CLIENT_WINDOW:
				return (WM_PARAM)pObj;
			case WM_GET_INSIDE_RECT:
				return DefaultProc(pObj, MsgId, Data);
		}
		return 0;
	}

public:
	MultPage(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget(r, Style | WC_LATE_CLIP, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE) {
		RECT rClient;
		_CalcClientRect(&rClient);
		pClient = new WObj(
			rClient,
			WC_VISIBLE | WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT | WC_ANCHOR_TOP | WC_ANCHOR_BOTTOM,
			_ClientCallback, this);
		_UpdatePositions();
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new MultPage(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, pWinParent, pCreateInfo->Id);
	}

public:

	void AddPage(WObj *pWin, const char *pText) {
		if (!pWin) {
			/* If we get no handle we must find it. To do this, we search      */
			/* all children until we found one that has not yet become a page. */
			auto pClient = this->pClient;
			for (auto pChild = pClient->FirstChild(); pChild && !pWin; pChild = pChild->NextSibling()) {
				pWin = pChild;
				for (int i = 0; i < Handles.NumItems(); i++) {
					auto &page = Handles[i];
					if (page.pWin == pChild) {
						pWin = nullptr;
						break;
					}
				}
			}
		}
		else {
			/* If we get a handle we must ensure that it was attached to the MultPage */
			pWin->Attach(this->pClient);
		}
		if (pWin) {
			Page page = {};
			char NullByte = 0;
			if (!pText) {
				pText = &NullByte;
			}
			page.pWin = pWin;
			page.Status = MULTIPAGE_STATE_ENABLED;
			if (Handles.AddItem(&page) == 0) {
				GUI__SetText(Handles[Handles.NumItems() - 1].pText, pText);
			}
			Selected(Handles.NumItems() - 1);
		}
	}
	void DeletePage(uint16_t Index, int Delete) {
		if (Index < Handles.NumItems()) {
			auto pWin = Handles[Index].pWin;
			/* Remove the page from the MultPage object */
			if (Index == sel) {
				if (Index == Handles.NumItems() - 1) {
					_ShowPage(Index - 1);
					sel--;
				}
				else
					_ShowPage(Index + 1);
			}
			else if (Index < sel)
				sel--;
			GUI_ALLOC_FreePtr((void **)&Handles[Index].pText);
			Handles.DeleteItem(Index);
			_UpdatePositions();
			/* Delete the window of the page */
			if (Delete)
				delete pWin;
		}
	}
	
	WObj *GetWindow(uint16_t Index) {
		if (Index < Handles.NumItems())
			return Handles[Index].pWin;
		return nullptr;
	}
	
	void PageEnable(uint16_t Index, bool bEnable) {
		_SetEnable(Index, bEnable);
		Invalidate();
	}
	bool PageEnable(uint16_t Index) const {
		if (Index < Handles.NumItems())
			return Handles[Index].Status & MULTIPAGE_STATE_ENABLED;
		return false;
	}

	void Selected(uint16_t Index) {
		if (Index < Handles.NumItems()) {
			if (Index != sel && (Handles[Index].Status & MULTIPAGE_STATE_ENABLED)) {
				_ShowPage(Index);
				sel = Index;
				_UpdatePositions();
			}
		}
	}	
	uint16_t Selected() const { return sel; }

	void Font(PCFONT pFont) {
		if (pFont) {
			Props.pFont = pFont;
			_UpdatePositions();
		}
	}

	void SetText(const char *pText, uint16_t Index) {
		if (Index < Handles.NumItems()) {
			if (GUI__SetText(Handles[Index].pText, pText))
				_UpdatePositions();
		}
	}

	void SetAlign(unsigned Align) {
		Props.Align = Align;
		RECT rClient;
		_CalcClientRect(&rClient);
		pClient->MoveTo(rClient.LeftTop() + Rect().LeftTop());
		_UpdatePositions();
	}

	void BkColor(RGBC Color, uint16_t Index) {
		if (Index < MULTIPAGE_NUMCOLORS) {
			Props.aBkColor[Index] = Color;
			Invalidate();
		}
	}

	void TextColor(RGBC Color, uint16_t Index) {
		if (Index < MULTIPAGE_NUMCOLORS) {
			Props.aTextColor[Index] = Color;
			Invalidate();
		}
	}
};

MultPage::Properties MultPage::DefaultProps;

}
