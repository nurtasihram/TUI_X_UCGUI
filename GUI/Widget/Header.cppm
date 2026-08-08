module;

#include "DIALOG_Intern.h"

export module TUX.Widget.Header;

import TUX.Widget;

import TUX.Array;

#define HEADER_SUPPORT_DRAG 1

/* Remember the old cursor */
static PCCURSOR _pOldCursor;

export {
   
struct HEADER_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		RGBC BkColor{ RGB_GRAYL(0xAA) };
		RGBC TextColor{ RGB_BLACK };
		PCCURSOR pCursor{ &CursorHeaderM };
		int16_t BorderH{ 0 };
		int16_t BorderV{ 2 };
	} static DefaultProps;
	Properties Props;
	struct Column {
		int16_t Width;
		TEXTALIGN Align;
		GUI_DRAW *pDrawObj;
		char *pText;
	};
	GUI_ARRAY_T<Column> Columns;
	int16_t CapturePosX = -1;
	int16_t CaptureItem = -1;
	uint16_t ScrollPos = 0;

	void _OnPaint() {
		int xPos = -ScrollPos;
		int NumItems = Columns.GetNumItems();
		int EffectSize = this->EffectSize();
		RECT Rect;
		GUI.SetBkColor(Props.BkColor);
		GUI_SetFont(Props.pFont);
		GUI_Clear();
		for (int i = 0; i < NumItems; i++) {
			auto pColumn = Columns.GetItem(i);
			Rect = WM_GetClientRect();
			Rect.x0 = xPos;
			Rect.x1 = Rect.x0 + pColumn->Width;
			if (auto pDraw = pColumn->pDrawObj) {
				int xOff = 0, yOff = 0;
				switch (pColumn->Align & TEXTALIGN_HORIZONTAL) {
					case TEXTALIGN_RIGHT:
						xOff = (pColumn->Width - pDraw->GetXSize());
						break;
					case TEXTALIGN_HCENTER:
						xOff = (pColumn->Width - pDraw->GetXSize()) / 2;
						break;
				}
				switch (pColumn->Align & TEXTALIGN_VERTICAL) {
					case TEXTALIGN_BOTTOM:
						yOff = ((Rect.y1 - Rect.y0 + 1) - pDraw->GetYSize());
						break;
					case TEXTALIGN_VCENTER:
						yOff = ((Rect.y1 - Rect.y0 + 1) - pDraw->GetYSize()) / 2;
						break;
				}
				WM_SetUserClipRect(&Rect);
				pDraw->Draw(xPos + xOff, yOff);
				WM_SetUserClipRect(nullptr);
			}
			DrawUp(Rect);
			xPos += Rect.x1 - Rect.x0;
			Rect.x0 += EffectSize + Props.BorderH;
			Rect.x1 -= EffectSize + Props.BorderH;
			Rect.y0 += EffectSize + Props.BorderV;
			Rect.y1 -= EffectSize + Props.BorderV;
			GUI.SetColor(Props.TextColor);
			GUI_DispStringInRect(pColumn->pText, &Rect, pColumn->Align);
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
		NumItems = Columns.GetNumItems();
		for (i = 0; i < NumItems; i++) {
			auto pColumn = Columns.GetItem(i);
			GUI_ALLOC_FreePtr((void **)&pColumn->pText);
			if (pColumn->pDrawObj) {
				GUI_ALLOC_Free(pColumn->pDrawObj);
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
			for (int Index = 0, NumColumns = Columns.GetNumItems(); Index < NumColumns; ++Index) {
				auto pColumn = Columns.GetItem(Index);
				xPos += pColumn->Width;
				if (x - 4 <= xPos && xPos <= x + 4) {
					if (Index < NumColumns && x < xPos) {
						pColumn = Columns.GetItem(Index);
						if (pColumn->Width)
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
			WM_SetCapture(this, 1);
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
				WM_ReleaseCapture();
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
		WM_NotifyParent(this, Notification);
	}
#endif
#if (HEADER_SUPPORT_DRAG & GUI_SUPPORT_MOUSE)
	void _OnMouseOver(const PID_STATE *pState) {
		if (pState)
			_HandlePID(pState->x + ScrollPos, pState->y, -1);
	}
#endif
	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (HEADER_Obj *)hWin;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
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

public:
	void SetFont(PCFONT pFont) {
		Props.pFont = pFont;
		WM_Invalidate(this);
	}
	void SetHeight(int Height) {
		auto Rect = WM_GetClientRect(this);
		WM_SetSize(this, Rect.x1 - Rect.x0 + 1, Height);
		WM_Invalidate(Parent());
	}
	void SetTextColor(RGBC Color) {
		Props.TextColor = Color;
		WM_Invalidate(this);
	}
	void SetBkColor(RGBC Color) {
		Props.BkColor = Color;
		WM_Invalidate(this);
	}
	void SetTextAlign(unsigned int Index, int Align) {
		if (Index <= Columns.GetNumItems()) {
			Column *pColumn;
			pColumn = Columns.GetItem(Index);
			pColumn->Align = Align;
			WM_Invalidate(this);
		}
	}
	void SetScrollPos(int ScrollPos) {
		if ((ScrollPos >= 0)) {
			if (ScrollPos != ScrollPos) {
				ScrollPos = ScrollPos;
				WM_Invalidate(this);
				WM_Invalidate(Parent());
			}
		}
	}
	void AddItem(int Width, const char *s, int Align) {
		Column Col = {};
		if (!Width) {
			PCFONT pFont = GUI_SetFont(Props.pFont);
			Width = GUI_GetStringDistX(s) + 2 * (this->EffectSize() + Props.BorderH);
			GUI_SetFont(pFont);
		}
		Col.Width = Width;
		Col.Align = Align;
		Col.pDrawObj = 0;
		int Index = Columns.GetNumItems();
		if (Columns.AddItem(&Col) == 0) {
			auto pColumn = Columns.GetItem(Index);
			GUI__SetText(&pColumn->pText, s);
			WM_Invalidate(this);
			WM_Invalidate(Parent());
		}
	}
	void DeleteItem(unsigned Index) {
		if (Index < Columns.GetNumItems()) {
			if (auto pColumn = Columns.GetItem(Index))
				GUI_ALLOC_FreePtr((void **)&pColumn->pText);
			Columns.DeleteItem(Index);
			WM_Invalidate(this);
			WM_Invalidate(Parent());
		}
	}
	void SetItemText(unsigned int Index, const char *s) {
		if (Index < Columns.GetNumItems()) {
			auto pColumn = Columns.GetItem(Index);
			if (pColumn && GUI__SetText(&pColumn->pText, s))
				WM_Invalidate(this);
		}
	}
	void SetItemWidth(unsigned int Index, int Width) {
		if ((Width >= 0)) {

			if (Index <= Columns.GetNumItems()) {
				auto pColumn = Columns.GetItem(Index);
				if (pColumn) {
					pColumn->Width = Width;
					WM_Invalidate(this);
					Parent()->Require(WM_NOTIFY_CLIENTCHANGE);
					WM_Invalidate(Parent());
				}
			}
		}
	}
	int GetHeight() {
		int Height = 0;
		RECT Rect = WM_GetClientRect(this);
		Rect -= Rect.LeftTop();
		Height = Rect.y1 - Rect.y0 + 1;

		return Height;
	}
	int GetItemWidth(unsigned int Index) {
		int Width = 0;
		if (Index <= Columns.GetNumItems()) {
			Column *pColumn;
			pColumn = Columns.GetItem(Index);
			Width = pColumn->Width;
		}

		return Width;
	}
	int  GetNumItems() {
		int NumCols = 0;
		NumCols = Columns.GetNumItems();
		return NumCols;
	}

	void _SetDrawObj(unsigned Index, GUI_DRAW *pDrawObj) {
		if (Index <= Columns.GetNumItems()) {
			auto pColumn = Columns.GetItem(Index);
			if (pColumn) {
				GUI_ALLOC_FreePtr((void **)&pColumn->pDrawObj);
				pColumn->pDrawObj = pDrawObj;
			}
		}
	}
	void SetBitmapEx(unsigned Index, PCBITMAP pBitmap, int x, int y) {
		_SetDrawObj(Index, GUI_DRAW_BITMAP_Create(pBitmap, x, y));
		WM_Invalidate(this);
	}
	void SetBitmap(unsigned Index, PCBITMAP pBitmap) {
		SetBitmapEx(Index, pBitmap, 0, 0);
	}
};

HEADER_Obj::Properties HEADER_Obj::DefaultProps;

HEADER_Obj *HEADER_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
							  int WinFlags, int ExFlags, int Id) {
	GUI_USE_PARA(ExFlags);
	/* Create the window */
	if ((xsize == 0) && (x0 == 0) && (y0 == 0)) {
		RECT Rect = WM_GetInsideRect(hParent);
		xsize = Rect.x1 - Rect.x0 + 1;
		x0 = Rect.x0;
		y0 = Rect.y0;
	}
	if (ysize == 0) {
		ysize = HEADER_Obj::DefaultProps.pFont->DistY();
		ysize += 2 * HEADER_Obj::DefaultProps.BorderV;
		ysize += 2 * WIDGET::DefaultEffect->EffectSize;
	}
	WinFlags |= WC_ANCHOR_LEFT | WC_ANCHOR_RIGHT;
	auto pObj = (HEADER_Obj *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, HEADER_Obj::_Callback,
								  sizeof(HEADER_Obj) - sizeof(WM_Obj));
	if (pObj) {
		/* init widget specific variables */
		WIDGET__Init(pObj, Id, 0);
		/* init member variables */
		pObj->Props = HEADER_Obj::DefaultProps;
		pObj->CapturePosX = -1;
		pObj->CaptureItem = -1;
		pObj->ScrollPos = 0;
	}
	else {
	}
	return pObj;
}

WM_Obj *HEADER_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	GUI_USE_PARA(cb);
	return HEADER_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							hWinParent, pCreateInfo->Flags, 0, pCreateInfo->Id);
}

}
