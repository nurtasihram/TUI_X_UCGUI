module;

#include "GUI_Protected.h"

export module TUX.Widget.Header;

import TUX.Widget;

import TUX.Array;

#define HEADER_SUPPORT_DRAG 1

/* Remember the old cursor */
static PCCURSOR _pOldCursor;

export {
   
class Header : public Widget {

public:
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		RGBC BkColor{ RGB_GRAYL(0xAA) };
		RGBC TextColor{ RGB_BLACK };
		PCCURSOR pCursor{ &CursorHeaderM };
		int16_t BorderH{ 0 };
		int16_t BorderV{ 2 };
	} static DefaultProps;
	
private:
	Properties Props = DefaultProps;
	
	struct Column {
		int16_t Width;
		TEXTALIGN Align;
		GUI_DRAW *pDrawObj;
		char *pText;
	};
	ARRAY<Column> Columns;
	int16_t CapturePosX = -1;
	int16_t CaptureItem = -1;
	uint16_t ScrollPos = 0;

	void _OnPaint() {
		int xPos = -ScrollPos;
		int NumItems = Columns.NumItems();
		int EffectSize = this->EffectSize();
		RECT Rect;
		GUI.SetBkColor(Props.BkColor);
		GUI.SetFont(Props.pFont);
		GUI_Clear();
		for (int i = 0; i < NumItems; i++) {
			auto &col = Columns[i];
			Rect = WM_GetClientRect();
			Rect.x0 = xPos;
			Rect.x1 = Rect.x0 + col.Width;
			if (auto pDraw = col.pDrawObj) {
				int xOff = 0, yOff = 0;
				switch (col.Align & TEXTALIGN_HORIZONTAL) {
					case TEXTALIGN_RIGHT:
						xOff = (col.Width - pDraw->GetXSize());
						break;
					case TEXTALIGN_HCENTER:
						xOff = (col.Width - pDraw->GetXSize()) / 2;
						break;
				}
				switch (col.Align & TEXTALIGN_VERTICAL) {
					case TEXTALIGN_BOTTOM:
						yOff = ((Rect.y1 - Rect.y0 + 1) - pDraw->GetYSize());
						break;
					case TEXTALIGN_VCENTER:
						yOff = ((Rect.y1 - Rect.y0 + 1) - pDraw->GetYSize()) / 2;
						break;
				}
				SetUserClipRect(&Rect);
				pDraw->Draw(RECT::LeftTop(Rect.LeftTop(), { xPos + xOff, yOff }));
				SetUserClipRect(nullptr);
			}
			DrawUp(Rect);
			xPos += Rect.x1 - Rect.x0;
			Rect.x0 += EffectSize + Props.BorderH;
			Rect.x1 -= EffectSize + Props.BorderH;
			Rect.y0 += EffectSize + Props.BorderV;
			Rect.y1 -= EffectSize + Props.BorderV;
			GUI.SetColor(Props.TextColor);
			GUI_DispStringInRect(col.pText, &Rect, col.Align);
		}
		Rect = WM_GetClientRect();
		Rect.x0 = xPos;
		Rect.x1 = 0xfff;
		DrawUp(Rect);
	}
	void _RestoreOldCursor(void) {
		if (_pOldCursor) {
#if GUI_SUPPORT_CURSOR
			GUI_CURSOR_Select(_pOldCursor);
#endif
			_pOldCursor = 0;
		}
	}
	void _FreeAttached() {
		int i, NumItems;
		NumItems = Columns.NumItems();
		for (i = 0; i < NumItems; i++) {
			auto &col = Columns[i];
			GUI_ALLOC_FreePtr((void **)&col.pText);
			if (col.pDrawObj) {
				GUI_ALLOC_Free(col.pDrawObj);
			}
		}
		/* Delete attached objects (if any) */
		Columns.Delete();
		_RestoreOldCursor();
	}
#if (HEADER_SUPPORT_DRAG)
	int _GetItemIndex(int x, int y) {
		if ((y >= 0) && (y < GetSizeY())) {
			int xPos = this->EffectSize();
			for (int Index = 0, NumColumns = Columns.NumItems(); Index < NumColumns; ++Index) {
				auto &col = Columns[Index];
				xPos += col.Width;
				if (x - 4 <= xPos && xPos <= x + 4) {
					if (Index < NumColumns && x < xPos) {
						if (Columns[Index].Width)
							return Index;
					}
				}
			}
		}
		return -1;
	}
	void _HandlePID(int x, int y, int Pressed) {
		int Hit = _GetItemIndex(x, y);
		/* set capture position () */
		if ((Pressed == 1) && (Hit >= 0) && (CapturePosX == -1)) {
			CapturePosX = x;
			CaptureItem = Hit;
		}
		/* set mouse cursor and capture () */
		if (Hit >= 0) {
			SetCapture(1);
#if GUI_SUPPORT_CURSOR
			if (!_pOldCursor) {
				_pOldCursor = GUI_CURSOR_Select(Props.pCursor);
			}
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
	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (Header *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_PAINT:
				pObj->_OnPaint();
				return 0;
#if (HEADER_SUPPORT_DRAG)
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
				return 0;
#endif
#if (HEADER_SUPPORT_DRAG & GUI_SUPPORT_MOUSE)
			case WM_MOUSEOVER:
				pObj->_OnMouseOver((const PID_STATE *)Data);
				return 0;
#endif
			case WM_DELETE:
				pObj->_FreeAttached(); /* No return here ... WM_DefaultProc needs to be called */
				return 0;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

private:
	static void _AdjRect(RECT &r, WObj *pParent) {
		auto Rect = pParent->GetInsideRect();
		if (r.x0 <= 0)
			r.x0 = Rect.x0;
		if (r.y0 <= 0)
			r.y0 = Rect.y0;
		if (r.x1 <= r.x0)
			r.x1 = Rect.x1;
		if (r.y1 <= r.y0)
			r.y1 = r.y0 + Header::DefaultProps.pFont->DistY()
				+ 2 * Header::DefaultProps.BorderV
				+ 2 * Widget::DefaultEffect->EffectSize;
	}
public:
	Header(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget((_AdjRect(r, pParent), r),
			   Style | WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT, _Callback, pParent, Id, 0) {}

public:

#pragma region Properties

	void SetFont(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		Invalidate();
	}

	void SetTextColor(RGBC Color) {
		if (Props.TextColor == Color)
			return;
		Props.TextColor = Color;
		Invalidate();
	}

	void SetBkColor(RGBC Color) {
		if (Props.BkColor == Color)
			return;
		Props.BkColor = Color;
		Invalidate();
	}

#pragma endregion

	int GetHeight() { return GetClientRect().YSize(); }
	void SetHeight(int Height) {
		SetSize({ GetSize().x, Height });
		Parent()->Invalidate();
	}

	void SetScrollPos(int ScrollPos) {
		if (ScrollPos >= 0) {
			if (ScrollPos != ScrollPos) {
				ScrollPos = ScrollPos;
				Invalidate();
				Parent()->Invalidate();
			}
		}
	}

	auto GetNumItems() { return Columns.NumItems(); }
	void AddItem(int Width, const char *s, int Align) {
		Column Col = {};
		if (!Width) {
			PCFONT pFont = GUI.SetFont(Props.pFont);
			Width = GUI_GetStringDistX(s) + 2 * (this->EffectSize() + Props.BorderH);
			GUI.SetFont(pFont);
		}
		Col.Width = Width;
		Col.Align = Align;
		Col.pDrawObj = 0;
		int Index = Columns.NumItems();
		if (Columns.AddItem(&Col) == 0) {
			auto &pColumn = Columns[Index];
			GUI__SetText(&pColumn.pText, s);
			Invalidate();
			Parent()->Invalidate();
		}
	}
	void DeleteItem(uint16_t Index) {
		if (Index < Columns.NumItems()) {
			GUI_ALLOC_FreePtr((void **)&Columns[Index].pText);
			Columns.DeleteItem(Index);
			Invalidate();
			Parent()->Invalidate();
		}
	}
	void SetItemText(uint16_t Index, const char *s) {
		if (Index < Columns.NumItems()) {
			if (GUI__SetText(&Columns[Index].pText, s))
				Invalidate();
		}
	}
	void SetItemWidth(uint16_t Index, int Width) {
		if (Width >= 0) {
			if (Index <= Columns.NumItems()) {
				auto &pColumn = Columns[Index];
				pColumn.Width = Width;
				Invalidate();
				Parent()->Require(WM_NOTIFY_CLIENTCHANGE);
				Parent()->Invalidate();
			}
		}
	}
	int16_t GetItemWidth(uint16_t Index) {
		if (Index < Columns.NumItems())
			return Columns[Index].Width;
		return 0;
	}
	void SetTextAlign(uint16_t Index, TEXTALIGN Align) {
		if (Index < Columns.NumItems()) {
			Columns[Index].Align = Align;
			Invalidate();
		}
	}

	void SetDrawObj(uint16_t Index, GUI_DRAW *pDrawObj) {
		if (Index < Columns.NumItems()) {
			auto &col = Columns[Index];
			GUI_ALLOC_FreePtr((void **)&col.pDrawObj);
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
