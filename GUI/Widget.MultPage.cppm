export module TUX.Widget.MultPage;

#include "GUIConf.h"

import TUX.Widget;
import TUX.Widget.ScrollBar;

import TUX.Array;

constexpr uint16_t
	MULTIPAGE_STATE_ENABLED     = 1 << 0,
	MULTIPAGE_STATE_SCROLLMODE  = WIDGET_STATE_USER<0>;

export {

constexpr uint16_t
	MULTIPAGE_ALIGN_LEFT   = 0 << 0,
	MULTIPAGE_ALIGN_RIGHT  = 1 << 0,
	MULTIPAGE_ALIGN_TOP    = 0 << 2,
	MULTIPAGE_ALIGN_BOTTOM = 1 << 2;

enum MULTIPAGE_CI {
	MULTIPAGE_CI_DISABLED = 0,
	MULTIPAGE_CI_ENABLED
};

class MultPage : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		BRUSH aBrush[2]{
			/* Index       | Background      | Text             */
			/* disabled */ { RGBC::Gray(0xD0), RGBC::Gray(0x80) },
			/* enabled  */ { RGBC::Gray(0xC0), RGB_BLACK        }
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
	
	RECT _CalcClientRect() const {
		auto r = _GetInsideRect();
		if (Props.Align & MULTIPAGE_ALIGN_BOTTOM)
			r.y1 -= Props.pFont->YSize + 6;
		else
			r.y0 += Props.pFont->YSize + 6;
		return r;
	}
	RECT _CalcBorderRect() const {
		auto r = ClientRect();
		if (Props.Align & MULTIPAGE_ALIGN_BOTTOM)
			r.y1 -= Props.pFont->YSize + 6;
		else
			r.y0 += Props.pFont->YSize + 6;
		return r;
	}
	uint16_t _GetPageSizeX(uint16_t Index) const {
		if (Index >= Handles.NumItems())
			return 0;
		return Props.pFont->TextBound(Handles[Index].pText).x + 10;
	}
	uint16_t _GetPagePosX(uint16_t Index) const {
		uint16_t r = 0;
		for (uint16_t i = 0; i < Index; i++)
			r += _GetPageSizeX(i);
		return r;
	}
	uint16_t _GetTextWidth() const {
		return _GetPagePosX(Handles.NumItems());
	}
	RECT _GetTextRect() const {
		RECT r;
		auto Height = Props.pFont->YSize + 6;
		auto rBorder = _CalcBorderRect();
		/* Calculate Y-Position of text item */
		r.y0 = Props.Align & MULTIPAGE_ALIGN_BOTTOM ? rBorder.y1 : 0;
		r.y1 = r.y0 + Height;
		/* Calculate width of text items */
		auto Width = States & MULTIPAGE_STATE_SCROLLMODE ? rBorder.x1 - ((Height * 3) >> 1) - 3 :  _GetTextWidth();
		/* Calculate X-Position of text item */
		if (Props.Align & MULTIPAGE_ALIGN_RIGHT) {
			r.x0 = rBorder.x1 - Width;
			r.x1 = rBorder.x1;
		}
		else {
			r.x0 = 0;
			r.x1 = Width;
		}
		return r;
	}

	void _UpdatePositions() {
		auto Width = _GetTextWidth();
		auto rBorder = _CalcBorderRect();
		/* Set scrollmode according to the text width */
		if (Width > rBorder.x1) {
			auto Size = ((Props.pFont->YSize + 6) * 3) >> 2;
			auto x0 = Props.Align & MULTIPAGE_ALIGN_RIGHT ? rBorder.x0 : rBorder.x1 - 2 * Size + 1;
			auto y0 = Props.Align & MULTIPAGE_ALIGN_BOTTOM ? rBorder.y1 : rBorder.y0 - Size + 1;
			/* A scrollbar is required so we add one to the MultPage */
			_AddScrollbar(x0, y0, 2 * Size, Size);
			auto rText = _GetTextRect();
			uint16_t NumItems = 0;
			while (Width >= Max(rText.XSize(), 1))
				Width -= _GetPageSizeX(NumItems++);
			_SetScrollbar(NumItems + 1);
		}
		else /* ScrollBar is no longer required. We delete it if there was one */
			_DeleteScrollbar();
		/* Move and resize the client area to the updated positions */
		rBorder = _CalcClientRect();
		pClient->MoveChildTo(rBorder.LeftTop());
		pClient->Size(rBorder.Size());
		Invalidate();
	}

	void _DrawTextItem(const char *pText, uint16_t Index,
					   const RECT *pRect, int x0, int w, int ColorIndex) const {
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
					GUI.DrawVLine(r.x0 - 1, r.y0, r.y0 + 1);
					GUI.Color(RGBC::Gray(0x55));
					GUI.DrawVLine(r.x1 + 1, r.y0, r.y0 + 1);
				}
			}
			else {
				r.y1 += EffectSize() + 1;
				if (EffectSize() > 1) {
					GUI.Color(RGB_WHITE);
					GUI.DrawVLine(r.x0 - 1, r.y1 - 2, r.y1 - 1);
					GUI.Color(RGBC::Gray(0x55));
					GUI.DrawVLine(r.x1 + 1, r.y1 - 2, r.y1 - 1);
				}
			}
		}
		GUI.Brush(Props.aBrush[ColorIndex]);
		GUI.FillRect(r);
		GUI_DispStringAt(pText, r.x0 + 4, pRect->y0 + 3);
	}
	void _OnPaint() const {
		SetBkColorPrefer(RGB_INVALID);
		GUI.Clear();
		/* Draw border of MultPage */
		auto rBorder = _CalcBorderRect();
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
		auto rText = _GetTextRect();
		auto rClip = rText;
		rClip.y0 = rText.y0 - 1;
		rClip.y1 = rText.y1 + 1;
		UserClip(&rClip);
		GUI.Font(Props.pFont);
		for (int i = 0; i < NumItems; i++) {
			auto &page = Handles[i];
			x0 += w;
			w = Props.pFont->TextBound(page.pText).x + 10;
			_DrawTextItem(page.pText, i, &rText, x0, w, (page.Status & MULTIPAGE_STATE_ENABLED) ? 1 : 0);
		}
		UserClip(nullptr);
	}
	bool _ClickedOnMultipage(POINT Pos) {
		auto NumItems = Handles.NumItems();
		if (!NumItems)
			return false;
		auto rText = _GetTextRect();
		if (!(rText <= Pos))
			return false;
		int w = 0, x0 = rText.x0;
		/* Check if another page must be selected */
		if (States & MULTIPAGE_STATE_SCROLLMODE)
			x0 -= _GetPagePosX(ScrollState);
		for (int i = 0; i < NumItems; i++) {
			x0 += w;
			w = _GetPageSizeX(i);
			if (x0 <= Pos.x && Pos.x <= x0 + w - 1) {
				Selected(i);
				NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
				return true;
			}
		}
		return false;
	}
	void _OnTouch(const PID_STATE *pState) {
		int Notification;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				POINT Pos = *pState;
				if (!_ClickedOnMultipage(Pos)) {
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
				*(RECT *)Data = pObj->_CalcClientRect();
				return 0;
			case WM_WIDGET_SET_EFFECT:
				if (auto pScroll = (ScrollBar *)pObj->GetScrollbarH())
					pScroll->SetEffect((const WIDGET_EFFECT *)Data);
			case WM_SIZE:
				pObj->_UpdatePositions();
				return 0;
			case WM_DELETE:
				for (int _i = 0, NumItems = pObj->Handles.NumItems(); _i < NumItems; _i++)
					GUI_MEM_FreePtr((void **)&pObj->Handles[_i].pText);
				pObj->Handles.Delete();
				return 0;
		}
		return pObj->WidgetProc(MsgId, Data);
	}
	static WM_PARAM _ClientCallback(WObj *pObj, int MsgId, WM_PARAM Data) {
		auto pParent = (MultPage *)pObj->Parent();
		switch (MsgId) {
			case WM_PAINT:
				GUI.BkColor(pParent->Props.aBrush[MULTIPAGE_CI_ENABLED].BkColor);
				GUI.Clear();
				return 0;
			case WM_GET_BKCOLOR:
				return (WM_PARAM)pParent->Props.aBrush[MULTIPAGE_CI_ENABLED].BkColor;
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
		Widget(r, Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE) {
		pClient = new WObj(
			_CalcClientRect(),
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

#pragma region Properties
	void Font(PCFONT pFont) {
		if (pFont) {
			Props.pFont = pFont;
			_UpdatePositions();
		}
	}

	void SetAlign(TEXTALIGN Align) {
		Props.Align = Align;
		auto rClient = _CalcClientRect();
		pClient->MoveTo(rClient.LeftTop() + Rect().LeftTop());
		_UpdatePositions();
	}

	void Brush(MULTIPAGE_CI Index, BRUSH brush) {
		if (Props.aBrush[Index] == brush)
			return;
		Props.aBrush[Index] = brush;
		Invalidate();
	}
#pragma endregion

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
		else /* If we get a handle we must ensure that it was attached to the MultPage */
			pWin->Attach(this->pClient);
		if (pWin) {
			Page page = {};
			char NullByte = 0;
			if (!pText)
				pText = &NullByte;
			page.pWin = pWin;
			page.Status = MULTIPAGE_STATE_ENABLED;
			if (Handles.AddItem(&page) == 0)
				GUI__SetText(Handles[Handles.NumItems() - 1].pText, pText);
			Selected(Handles.NumItems() - 1);
		}
	}
	void DeletePage(uint16_t Index, int Delete) {
		if (Index >= Handles.NumItems())
			return;
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
		GUI_MEM_FreePtr((void **)&Handles[Index].pText);
		Handles.DeleteItem(Index);
		_UpdatePositions();
		/* Delete the window of the page */
		if (Delete)
			delete pWin;
	}
	void PageEnable(uint16_t Index, bool bEnable) {
		if (Index >= Handles.NumItems())
			return;
		auto &page = Handles[Index];
		if (bEnable)
			page.Status |= MULTIPAGE_STATE_ENABLED;
		else
			page.Status &= ~MULTIPAGE_STATE_ENABLED;
		Invalidate();
	}
	bool PageEnable(uint16_t Index) const {
		if (Index >= Handles.NumItems())
			return false;
		return Handles[Index].Status & MULTIPAGE_STATE_ENABLED;
	}
	WObj *GetWindow(uint16_t Index) {
		if (Index >= Handles.NumItems())
			return nullptr;
		return Handles[Index].pWin;
	}
	void SetText(const char *pText, uint16_t Index) {
		if (Index < Handles.NumItems()) {
			if (GUI__SetText(Handles[Index].pText, pText))
				_UpdatePositions();
		}
	}

	void Selected(uint16_t Index) {
		if (Index >= Handles.NumItems())
			return;
		if (Index != sel && (Handles[Index].Status & MULTIPAGE_STATE_ENABLED)) {
			_ShowPage(Index);
			sel = Index;
			_UpdatePositions();
		}
	}	
	uint16_t Selected() const { return sel; }
};

MultPage::Properties MultPage::DefaultProps;

}
