module;

#include "GUI_Protected.h"

export module TUX.Widget.ScrollBar;

import TUX.Widget;

export {
constexpr uint16_t SCROLLBAR_CF_FOCUSSABLE   = WIDGET_STATE_FOCUSSABLE;
constexpr uint16_t SCROLLBAR_CF_VERTICAL     = WIDGET_STATE_USER<0>;
constexpr uint16_t SCROLLBAR_STATE_PRESSED   = WIDGET_STATE_USER<1>;

struct SCROLLBAR_POSITIONS {
	int16_t x0_LeftArrow = 0,
			x1_LeftArrow = 0;
	int16_t x0_RightArrow = 0,
			x1_RightArrow = 0;
	int16_t x0_Thumb = 0,
			x1_Thumb = 0;
	int16_t ThumbSize = 0;
	int16_t xSizeMoveable = 0;
	int16_t x1 = 0;
};

class ScrollBar : public Widget {
	
public:
	struct Properties {
		RGBC aBkColor[2]{
			RGB_GRAYL(0x80),
			RGB_BLACK
		};
		RGBC Color{ RGB_GRAYL(0xC0) };
	} static DefaultProps;
	static const int16_t DefaultWidth = 12;

private:
	Properties Props = DefaultProps;

	WM_SCROLL_STATE ScrollState{ 100, 10, 0 };

	int _GetArrowSize() {
		auto Size = GetSize();
		if (GetStates() & SCROLLBAR_CF_VERTICAL)
			Size = ~Size;
		auto r = Size.y / 2 + 5;
		if (r > Size.x - 5)
			r = Size.x - 5;
		return r;
	}
	SCROLLBAR_POSITIONS _CalcPositions() {
		SCROLLBAR_POSITIONS Pos;
		auto r = Rect;
		Pos.x1 = (GetStates() & SCROLLBAR_CF_VERTICAL) ? r.y1 : r.x1;
		/* Subtract the rectangle of the other scrollbar (if existing and visible) */
		switch (GetId()) {
		case GUI_ID_HSCROLL:
			if (auto pWin = pParent->GetScrollbarV()) {
				auto rSub = pWin->GetRect();
				if (r.x1 == rSub.x1)
					r.x1 = rSub.x0 - 1;
			}
			break;
		case GUI_ID_VSCROLL:
			if (auto pWin = pParent->GetScrollbarH()) {
				auto rSub = pWin->GetRect();
				if (r.y1 == rSub.y1)
					r.y1 = rSub.y0 - 1;
			}
			break;
		}
		/* Convert coordinates of this window */
		r -= r.LeftTop();
		/* Convert real into virtual coordinates */
		if (GetStates() & SCROLLBAR_CF_VERTICAL)
			r = r.Rotate90R(GetSizeY());
		auto NumItems = ScrollState.NumItems;
		auto xSize = r.x1 - r.x0 + 1;
		auto xSizeArrow = _GetArrowSize();
		auto xSizeThumbArea = xSize - 2 * xSizeArrow;     /* Number of pixels available for thumb and movement */
		auto ThumbSize = GUI__DivideRound(xSizeThumbArea * ScrollState.PageSize, NumItems);
		if (ThumbSize < 4)
			ThumbSize = 4;
		if (ThumbSize > xSizeThumbArea)
			ThumbSize = xSizeThumbArea;
		auto xSizeMoveable = xSizeThumbArea - ThumbSize;
		Pos.x0_LeftArrow = r.x0;
		Pos.x1_LeftArrow = xSizeArrow - 1;
		Pos.x1_RightArrow = xSize - 1;
		Pos.x0_RightArrow = xSize - xSizeArrow;
		Pos.x0_Thumb = Pos.x1_LeftArrow + 1 + GUI__DivideRound(xSizeMoveable * ScrollState.v, NumItems - ScrollState.PageSize);
		Pos.x1_Thumb = Pos.x0_Thumb + ThumbSize - 1;
		Pos.xSizeMoveable = xSizeMoveable;
		Pos.ThumbSize = ThumbSize;
		return Pos;
	}
	void _DrawTriangle(int x, int y, int Size, int Inc) {
		if (GetStates() & SCROLLBAR_CF_VERTICAL)
			for (; Size >= 0; Size--, x += Inc)
				GUI_DrawHLine(x, y - Size, y + Size);
		else
			for (; Size >= 0; Size--, x += Inc)
				GUI_DrawVLine(x, y - Size, y + Size);
	}
	void _OnPaint() {
		/*
		  Get / calc position info
		*/
		auto r = GetClientRect();
		auto Pos = _CalcPositions();
		auto Height = GetStates() & SCROLLBAR_CF_VERTICAL ? r.DistX() : r.DistY();
		auto CenterH = Height >> 1;
		auto ArrowSize = (Height / 3) - 1;
		auto ArrowOff = 3 + ArrowSize + ArrowSize / 3;
		int16_t RECT:: *x0, RECT:: *x1;
		if (GetStates() & SCROLLBAR_CF_VERTICAL)
			x0 = &RECT::y0, x1 = &RECT::y1;
		else
			x0 = &RECT::x0, x1 = &RECT::x1;
		/*
		  Draw left Arrow
		*/
		GUI.Color(Props.Color);
		r.*x0 = Pos.x0_LeftArrow;
		r.*x1 = Pos.x1_LeftArrow;
		GUI_FillRect(r);
		GUI.Color(Props.aBkColor[1]);
		_DrawTriangle(r.*x0 + ArrowOff, CenterH, ArrowSize, -1);
		DrawUp(r);
		/*
		  Draw the thumb area which is not covered by the thumb
		*/
		GUI.Color(Props.aBkColor[0]);
		r.*x0 = Pos.x1_LeftArrow + 1;
		r.*x1 = Pos.x0_Thumb - 1;
		GUI_FillRect(r);
		r.*x0 = Pos.x1_Thumb + 1;
		r.*x1 = Pos.x0_RightArrow - 1;
		GUI_FillRect(r);
		/*
		  Draw Thumb
		*/
		r.*x0 = Pos.x0_Thumb;
		r.*x1 = Pos.x1_Thumb;
		GUI.Color(Props.Color);
		GUI_FillRect(r);
		DrawUp(r);
		/*
		  Draw right Arrow
		*/
		GUI.Color(Props.Color);
		r.*x0 = Pos.x0_RightArrow;
		r.*x1 = Pos.x1_RightArrow;
		GUI_FillRect(r);
		GUI.Color(Props.aBkColor[1]);
		_DrawTriangle(r.*x1 - ArrowOff, CenterH, ArrowSize, 1);
		DrawUp(r);
		/*
		  Draw overlap area (if any ...)
		*/
		if (Pos.x1_RightArrow != Pos.x1) {
			r.*x0 = Pos.x1_RightArrow + 1;
			r.*x1 = Pos.x1;
			GUI.Color(Props.Color);
			GUI_FillRect(r);
		}
	}
	void _ScrollbarPressed() {
		AddStates(SCROLLBAR_STATE_PRESSED);
		if (Status & WC_VISIBLE)
			NotifyParent(WM_NOTIFICATION_CLICKED);
	}
	void _ScrollbarReleased() {
		DelStates(SCROLLBAR_STATE_PRESSED);
		if (Status & WC_VISIBLE)
			NotifyParent(WM_NOTIFICATION_RELEASED);
	}
	void _OnTouch(const PID_STATE *pState) {
		if (!pState)
			return;
		if (!pState->Pressed) {
			/* React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown) */
			if (GetStates() & SCROLLBAR_STATE_PRESSED)
				_ScrollbarReleased();
			return;
		}
		auto Sel = ScrollState.v;
		auto Pos = _CalcPositions();
		auto Range = ScrollState.NumItems - ScrollState.PageSize;
		/* Swap mouse coordinates if necessary */
		int x = GetStates() & SCROLLBAR_CF_VERTICAL ? pState->y : pState->x;
		if (x <= Pos.x1_LeftArrow) /* left arrow (line left) */
			Sel--;
		else if (x < Pos.x0_Thumb) /* left area  (page left) */
			Sel -= ScrollState.PageSize;
		else if (x <= Pos.x1_Thumb) {      /* Thumb area */
			if (Pos.xSizeMoveable > 0) {
				x = x - Pos.ThumbSize / 2 - Pos.x1_LeftArrow - 1;
				Sel = GUI__DivideRound(Range * x, Pos.xSizeMoveable);
			}
		}
		else if (x < Pos.x0_RightArrow) /* right area (page right) */
			Sel += ScrollState.PageSize;
		else  if (x <= Pos.x1_RightArrow)
			Sel++;
		/* hObj->SetFocus(); */
		SetCapture(1);
		SetValue(Sel);
		if (!(GetStates() & SCROLLBAR_STATE_PRESSED))
			_ScrollbarPressed();
	}
	char _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0) {
			switch (pInfo->Key) {
				case GUI_KEY_RIGHT:
				case GUI_KEY_DOWN:
					Inc();
					return 1; /* Send to parent by not doing anything */
				case GUI_KEY_LEFT:
				case GUI_KEY_UP:
					Dec();
					return 1; /* Send to parent by not doing anything */
			}
		}
		return 0;
	}
	void _OnSetScrollState(const WM_SCROLL_STATE *pState) {
		if (ScrollState != *pState) {
			ScrollState = *pState;
			Invalidate();
		}
	}

	void _InvalidatePartner() { /* Invalidate the partner, since it is also affected */
		if (auto pScroll = WM_GetScrollPartner(this))
			pScroll->Invalidate();
		Parent()->Require(WM_NOTIFY_CLIENTCHANGE);   /* Client area may have changed */
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (ScrollBar *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
			case WM_DELETE:
				pObj->_InvalidatePartner();
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0; /* Send to parent by not doing anything */
				break;
			case WM_SET_SCROLL_STATE:
				pObj->_OnSetScrollState((const WM_SCROLL_STATE *)Data);
				return 0;
			case WM_GET_SCROLL_STATE:
				*(WM_SCROLL_STATE *)Data = pObj->ScrollState;
				break;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

private:
	static void _AdjRect(RECT &r, WObj *pParent, bool bVertical) {
		if (r.x1 > r.x0 && r.y1 > r.y0)
			return;
		auto Rect = pParent->GetInsideRect();
		if (bVertical) {
			r.y0 = Rect.y0;
			r.y1 = Rect.y1;
			if (r.x1 <= r.x0)
				r.x1 = Rect.x1;
			r.x0 = r.x1 - ScrollBar::DefaultWidth;
		}
		else {
			r.x0 = Rect.x0;
			r.x1 = Rect.x1;
			if (r.y1 <= r.y0)
				r.y1 = Rect.x1;
			r.y0 = r.y1 - ScrollBar::DefaultWidth;
		}
	}
public:
	ScrollBar(RECT r, WM_CF Style, WObj *pParent, uint16_t Id,
			  uint16_t ExFlags) :
		Widget((_AdjRect(r, pParent, ExFlags & SCROLLBAR_CF_VERTICAL), r),
			   Style, _Callback, pParent, Id,
			   Id != GUI_ID_HSCROLL && Id != GUI_ID_VSCROLL ?
			   ExFlags | WIDGET_STATE_FOCUSSABLE : ExFlags) {
		_InvalidatePartner();
	}
	ScrollBar(WObj *pParent, int SpecialFlags) :
		ScrollBar({},
				  WC_VISIBLE | WC_STAYONTOP | WC_ANCHOR_RIGHT | WC_ANCHOR_BOTTOM |
				  (SpecialFlags & SCROLLBAR_CF_VERTICAL ? WC_ANCHOR_TOP : WC_ANCHOR_LEFT),
				  pParent, SpecialFlags &SCROLLBAR_CF_VERTICAL ? GUI_ID_VSCROLL : GUI_ID_HSCROLL, SpecialFlags) {
		NotifyParent(WM_NOTIFICATION_SCROLLBAR_ADDED);
	}

public:

	void Dec() {
		AddValue(-1);
	}
	void Inc() {
		AddValue(1);
	}
	void AddValue(int Add) {
		SetValue(ScrollState.v + Add);
	}
	void SetValue(int v) {
		int Max = ScrollState.NumItems - ScrollState.PageSize;
		if (Max < 0)
			Max = 0;
		/* Put in min/max range */
		if (v < 0)
			v = 0;
		if (v > Max)
			v = Max;
		if (ScrollState.v != v) {
			ScrollState.v = v;
			Invalidate();
			NotifyParent(WM_NOTIFICATION_VALUE_CHANGED);
		}
	}
	void SetNumItems(int NumItems) {
		if (ScrollState.NumItems != NumItems) {
			ScrollState.NumItems = NumItems;
			Invalidate();
		}
	}
	void SetPageSize(int PageSize) {
		if (ScrollState.PageSize != PageSize) {
			ScrollState.PageSize = PageSize;
			Invalidate();
		}
	}
	void SetState(const WM_SCROLL_STATE *pState) {
		SetPageSize(pState->PageSize);
		SetNumItems(pState->NumItems);
		SetValue(pState->v);
	}
	int  GetValue() {
		return ScrollState.v;
	}
	void SetWidth(int Width) {
		POINT Size{ ScrollState.PageSize, Width };
		if (GetStates() & SCROLLBAR_CF_VERTICAL)
			Size = ~Size;
		SetSize(Size);
		_InvalidatePartner(); /* Invalidate the partner, since it is also affected */
	}

};

ScrollBar::Properties ScrollBar::DefaultProps;

}
