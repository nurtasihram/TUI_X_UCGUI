export module TUX.Widget.Header;

#include "GUIConf.h"

import TUX.Widget;

import TUX.Array;

#define HEADER_SUPPORT_DRAG 1

/* Remember the old cursor */
static PCCURSOR _pOldCursor;

export {
   
class Header : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
		BRUSH brush{ RGBC::Gray(0xAA), RGB_BLACK };
		PCCURSOR pCursor{ &CursorHeaderM };
		POINT Border{ 0, 2 };
	} static DefaultProps;

private:
	Properties Props = DefaultProps;
	
	struct Column {
		uint16_t Width = 0;
		TEXTALIGN Align = TEXTALIGN_LEFT;
		GUI_DRAW *pDrawObj = nullptr;
		char *pText = nullptr;
	};
	ARRAY<Column> Columns;
	int16_t CapturePosX = -1;
	int16_t CaptureItem = -1;
	uint16_t ScrollPos = 0;

	~Header() {
		for (uint16_t i = 0, NumItems = Columns.NumItems(); i < NumItems; i++) {
			auto &col = Columns[i];
			GUI_MEM_Free(col.pText);
			if (col.pDrawObj)
				GUI_MEM_Free(col.pDrawObj);
		}
		/* Delete attached objects (if any) */
		Columns.Delete();
		_RestoreOldCursor();
	}

	void _OnPaint() const {
		auto xPos = -ScrollPos;
		auto EffectSize = this->EffectSize();
		GUI.BkColor(Props.brush.BkColor);
		GUI.Font(Props.pFont);
		GUI.Clear();
		for (uint16_t i = 0, NumItems = Columns.NumItems(); i < NumItems; i++) {
			auto &col = Columns[i];
			auto r = ClientRect();
			r.x0 = xPos;
			r.x1 = r.x0 + col.Width;
			if (auto pDraw = col.pDrawObj) {
				POINT Size = pDraw->Size(), Off;
				switch (col.Align & TEXTALIGN_HORIZONTAL) {
					case TEXTALIGN_RIGHT:
						Off.x = (col.Width - Size.x);
						break;
					case TEXTALIGN_HCENTER:
						Off.x = (col.Width - Size.x) / 2;
						break;
				}
				switch (col.Align & TEXTALIGN_VERTICAL) {
					case TEXTALIGN_BOTTOM:
						Off.y = (r.YSize() - Size.y);
						break;
					case TEXTALIGN_VCENTER:
						Off.y = (r.YSize() - Size.y) / 2;
						break;
				}
				Off.x += xPos;
				UserClip(&r);
				pDraw->Draw(RECT::LeftTop(r.LeftTop(), Off));
				UserClip(nullptr);
			}
			DrawUp(r);
			xPos += r.DistX();
			r /= EffectSize;
			r += Props.Border;
			GUI.Brush(Props.brush);
			GUI_DispStringInRect(col.pText, r, col.Align);
		}
		auto r = ClientRect();
		r.x0 = xPos;
		r.x1 = 0xfff;
		DrawUp(r);
	}
	void _RestoreOldCursor(void) {
		if (_pOldCursor) {
#if GUI_SUPPORT_CURSOR
			GUI_CURSOR_Select(_pOldCursor);
#endif
			_pOldCursor = 0;
		}
	}
#if (HEADER_SUPPORT_DRAG)
	int _GetItemIndex(int x, int y) {
		if ((y >= 0) && (y < SizeY())) {
			int xPos = this->EffectSize();
			for (int i = 0, n = Columns.NumItems(); i < n; ++i) {
				auto &col = Columns[i];
				xPos += col.Width;
				if (x - 4 <= xPos && xPos <= x + 4)
					if (col.Width)
						return i;
			}
		}
		return -1;
	}
	void _HandlePID(int x, int y, int Pressed) {
		auto Hit = _GetItemIndex(x, y);
		/* set capture position () */
		if ((Pressed == 1) && (Hit >= 0) && (CapturePosX == -1)) {
			CapturePosX = x;
			CaptureItem = Hit;
		}
		/* set mouse cursor and capture () */
		if (Hit >= 0) {
			SetCapture(1);
#if GUI_SUPPORT_CURSOR
			if (!_pOldCursor)
				_pOldCursor = GUI_CURSOR_Select(Props.pCursor);
#endif
		}
		/* modify header */
		if ((CapturePosX >= 0) && (x != CapturePosX) && (Pressed == 1)) {
			int NewSize = GetItemWidth(CaptureItem) + x - CapturePosX;
			if (NewSize >= 0) {
				SetItemWidth(CaptureItem, NewSize);
				CapturePosX = x;
			}
		}
		/* release capture & restore cursor */
		if (Pressed <= 0) {
#if (GUI_SUPPORT_MOUSE)
			if (Hit == -1)
#endif
			{
				_RestoreOldCursor();
				CapturePosX = -1;
				ReleaseCapture();
			}
		}
	}
	void _OnTouch(const PID_STATE *pState) {
		int Notification;
		if (pState) {  /* Something happened in our area (pressed or released) */
			_HandlePID(pState->x + ScrollPos, pState->y, pState->Pressed);
			Notification = pState->Pressed ?
				WM_NOTIFICATION_CLICKED : WM_NOTIFICATION_RELEASED;
		}
		else
			Notification = WM_NOTIFICATION_MOVED_OUT;
		NotifyParent(Notification);
	}
#endif
#if (HEADER_SUPPORT_DRAG & GUI_SUPPORT_MOUSE)
	void _OnMouseOver(const PID_STATE *pState) {
		if (pState)
			_HandlePID(pState->x + ScrollPos, pState->y, -1);
	}
#endif
	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Header *)pWin;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
#if (HEADER_SUPPORT_DRAG)
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
#if (GUI_SUPPORT_MOUSE)
			case WM_MOUSEOVER:
				pObj->_OnMouseOver((const PID_STATE *)Data);
				return 0;
#endif
#endif
			case WM_DELETE:
				pObj->~Header();
				return 0;
		}
		return pObj->WidgetProc(MsgId, Data);
	}

private:
	static void _AdjRect(RECT &r, WObj *pParent) {
		auto rInside = pParent->InsideRect();
		if (r.x0 <= 0)
			r.x0 = rInside.x0;
		if (r.y0 <= 0)
			r.y0 = rInside.y0;
		if (r.x1 <= r.x0)
			r.x1 = rInside.x1;
		if (r.y1 <= r.y0)
			r.y1 = r.y0 + Header::DefaultProps.pFont->YSize
				+ 2 * Header::DefaultProps.Border.y
				+ 2 * Widget::DefaultEffect->EffectSize;
	}
public:
	Header(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget((_AdjRect(r, pParent), r),
			   Style | WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT, _Callback, pParent, Id, 0) {}

public:

#pragma region Properties
	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}

	void Brush(BRUSH brush) {
		if (Props.brush == brush)
			return;
		Props.brush = brush;
		Invalidate();
	}
#pragma endregion

	auto GetHeight() const { return ClientRect().YSize(); }
	void SetHeight(int Height) {
		Size({ SizeX(), Height });
		Parent()->Invalidate();
	}

	void SetScrollPos(uint16_t ScrollPos) {
		if (this->ScrollPos != ScrollPos) {
			this->ScrollPos = ScrollPos;
			Invalidate();
			Parent()->Invalidate();
		}
	}

	auto GetNumItems() const { return Columns.NumItems(); }
	void AddItem(uint16_t Width, const char *s, int Align) {
		Column Col;
		Col.Width = Width ? Width : 
			Props.pFont->TextBound(s).x + 2 * (EffectSize() + Props.Border.x);
		Col.Align = Align;
		auto Index = Columns.NumItems();
		if (Columns.AddItem(&Col) == 0) {
			auto &pColumn = Columns[Index];
			GUI__SetText(pColumn.pText, s);
			Invalidate();
			Parent()->Invalidate();
		}
	}
	void DeleteItem(uint16_t Index) {
		if (Index < Columns.NumItems()) {
			GUI_MEM_FreePtr((void **)&Columns[Index].pText);
			Columns.Delete(Index);
			Invalidate();
			Parent()->Invalidate();
		}
	}
	void SetItemText(uint16_t Index, const char *s) {
		if (Index < Columns.NumItems()) {
			if (GUI__SetText(Columns[Index].pText, s))
				Invalidate();
		}
	}
	void SetItemWidth(uint16_t Index, uint16_t Width) {
		if (Index < Columns.NumItems()) {
			Columns[Index].Width = Width;
			Invalidate();
			Parent()->Require(WM_NOTIFY_CLIENTCHANGE);
			Parent()->Invalidate();
		}
	}
	uint16_t GetItemWidth(uint16_t Index) {
		if (Index < Columns.NumItems())
			return Columns[Index].Width;
		return 0;
	}
	void TextAlign(uint16_t Index, TEXTALIGN Align) {
		if (Index < Columns.NumItems()) {
			Columns[Index].Align = Align;
			Invalidate();
		}
	}

	void SetDrawObj(uint16_t Index, GUI_DRAW *pDrawObj) {
		if (Index < Columns.NumItems()) {
			auto &col = Columns[Index];
			GUI_MEM_FreePtr((void **)&col.pDrawObj);
			col.pDrawObj = pDrawObj;
		}
	}
	void SetBitmapEx(uint16_t Index, PCBITMAP pBitmap) {
		SetDrawObj(Index, GUI_DRAW_BITMAP_Create(pBitmap));
		Invalidate();
	}
	void SetBitmap(uint16_t Index, PCBITMAP pBitmap) {
		SetBitmapEx(Index, pBitmap);
	}
};

Header::Properties Header::DefaultProps;

}
