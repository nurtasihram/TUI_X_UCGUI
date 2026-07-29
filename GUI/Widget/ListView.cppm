module;

#include "DIALOG_Intern.h"

export module TUX.Widget.ListView;

import TUX.Widget;
import TUX.Widget.Header;
import TUX.Widget.ScrollBar;

import TUX.Array;

export {
enum LISTVIEW_CI {
	 LISTVIEW_CI_UNSEL     = 0,
	 LISTVIEW_CI_SEL       = 1,
	 LISTVIEW_CI_SELFOCUS  = 2
};

struct LISTVIEW_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &FontProp13_1 };
		RGBC aBkColor[3]{
			/* Not selected */       RGB_WHITE,
			/* Selected, no focus */ RGB_GRAY,
			/* Selected, focus */    RGB_DARKBLUE
		};
		RGBC aTextColor[3]{
			/* Not selected */       RGB_BLACK,
			/* Selected, no focus */ RGB_WHITE,
			/* Selected, focus */    RGB_WHITE
		};
		RGBC GridColor{ RGB_LIGHTGRAY };
	} static DefaultProps;
	Properties Props;
	struct ItemInfo {
		RGBC aBkColor[3];
		RGBC aTextColor[3];
	};
	struct Item {
		WM_HMEM hItemInfo;
		char acText[1];
	};
	HEADER_Obj *pHeader;
	GUI_ARRAY   RowArray;         /* One entry per line. Every entry is a handle of GUI_ARRAY of strings */
	GUI_ARRAY   AlignArray;       /* One entry per column */
	int16_t     Sel;
	bool        ShowGrid;
	uint16_t    RowDistY, LBorder, RBorder;
	WM_SCROLL_STATE ScrollStateV, ScrollStateH;
	WM_Obj *hOwner;

	void _NotifyOwner(int Notification) {
		auto hOwner = this->hOwner ? this->hOwner : WM_GetParent(this);
		WM_NOTIFY_INFO Info;
		Info.Notification = Notification;
		Info.pWinSrc = this;
		WM_SendMessage(hOwner, WM_NOTIFY_PARENT, (WM_PARAM)&Info);
	}

	unsigned _GetRowDistY() {
		unsigned RowDistY;
		if (this->RowDistY) {
			RowDistY = this->RowDistY;
		}
		else {
			RowDistY = GUI_GetYDistOfFont(Props.pFont);
			if (this->ShowGrid) {
				RowDistY++;
			}
		}
		return RowDistY;
	}
	unsigned _GetNumVisibleRows() {
		unsigned RowDistY, ySize, r = 1;
		GUI_RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		ySize = Rect.y1 - Rect.y0 + 1 - pHeader->GetHeight();
		RowDistY = _GetRowDistY();
		if (RowDistY) {
			r = ySize / RowDistY;
			r = (r == 0) ? 1 : r;
		}
		return r;
	}
	void _OnPaint(const GUI_RECT *pClipRect) {
		const GUI_ARRAY *pRow;
		GUI_RECT ClipRect, Rect;
		int NumRows, NumVisRows, NumColumns;
		int LBorder, RBorder, EffectSize;
		int xPos, yPos, Width, RowDistY;
		int Align, i, j, EndRow;
		/* Init some values */
		NumColumns = pHeader->GetNumItems();
		NumRows = GUI_ARRAY_GetNumItems(&this->RowArray);
		NumVisRows = _GetNumVisibleRows();
		RowDistY = _GetRowDistY();
		LBorder = this->LBorder;
		RBorder = this->RBorder;
		EffectSize = this->pEffect->EffectSize;
		yPos = pHeader->GetHeight() + EffectSize;
		EndRow = this->ScrollStateV.v + (((NumVisRows + 1) > NumRows) ? NumRows : NumVisRows + 1);
		/* Calculate clipping rectangle */
		ClipRect = *pClipRect - this->Rect.LeftTop();
		WM_GetInsideRectExScrollbar(this, &Rect);
		ClipRect &= Rect;
		/* Set drawing color, font and text mode */
		GUI_SetColor(Props.aTextColor[0]);
		GUI_SetFont(Props.pFont);
		GUI_SetTextMode(DRAWMODE_TRANS);
		/* Do the drawing */
		for (i = this->ScrollStateV.v; i < EndRow; i++) {
			pRow = (const GUI_ARRAY *)GUI_ARRAY_GetpItem(&this->RowArray, i);
			if (pRow) {
				Rect.y0 = yPos;
				/* Break when all other rows are outside the drawing area */
				if (Rect.y0 > ClipRect.y1) {
					break;
				}
				Rect.y1 = yPos + RowDistY - 1;
				/* Make sure that we draw only when row is in drawing area */
				if (Rect.y1 >= ClipRect.y0) {
					int ColorIndex;
					/* Set background color */
					if (i == this->Sel) {
						ColorIndex = (this->State & WIDGET_STATE_FOCUS) ? 2 : 1;
					}
					else {
						ColorIndex = 0;
					}
					GUI_SetBkColor(Props.aBkColor[ColorIndex]);
					/* Iterate over all columns */
					if (this->ShowGrid) {
						Rect.y1--;
					}
					xPos = EffectSize - this->ScrollStateH.v;
					for (j = 0; j < NumColumns; j++) {
						Width = pHeader->GetItemWidth(j);
						Rect.x0 = xPos;
						/* Break when all other columns are outside the drawing area */
						if (Rect.x0 > ClipRect.x1) {
							break;
						}
						Rect.x1 = xPos + Width - 1;
						/* Make sure that we draw only when column is in drawing area */
						if (Rect.x1 >= ClipRect.x0) {
							Item *pItem;
							pItem = (Item *)GUI_ARRAY_GetpItem(pRow, j);
							if (pItem->hItemInfo) {
								ItemInfo *pItemInfo;
								pItemInfo = (ItemInfo *)(pItem->hItemInfo);
								GUI_SetBkColor(pItemInfo->aBkColor[ColorIndex]);
								GUI_SetColor(pItemInfo->aTextColor[ColorIndex]);
							}
							else {
								GUI_SetColor(Props.aTextColor[ColorIndex]);
							}
							/* Clear background */
							GUI_ClearRect(Rect);
							/* Draw text */
							Rect.x0 += LBorder;
							Rect.x1 -= RBorder;
							Align = *((int *)GUI_ARRAY_GetpItem(&this->AlignArray, j));
							GUI_DispStringInRect(pItem->acText, &Rect, Align);
							if (pItem->hItemInfo)
								GUI_SetBkColor(Props.aBkColor[ColorIndex]);
						}
						xPos += Width;
					}
					/* Clear unused area to the right of items */
					if (xPos <= ClipRect.x1)
						GUI_ClearRect({ xPos, Rect.y0, ClipRect.x1, Rect.y1 });
				}
				yPos += RowDistY;
			}
		}
		/* Clear unused area below items */
		if (yPos <= ClipRect.y1) {
			GUI_SetBkColor(Props.aBkColor[0]);
			GUI_ClearRect({ ClipRect.x0, yPos, ClipRect.x1, ClipRect.y1 });
		}
		/* Draw grid */
		if (this->ShowGrid) {
			GUI_SetColor(Props.GridColor);
			yPos = pHeader->GetHeight() + EffectSize - 1;
			for (i = 0; i < NumVisRows; i++) {
				yPos += RowDistY;
				/* Break when all other rows are outside the drawing area */
				if (yPos > ClipRect.y1) {
					break;
				}
				/* Make sure that we draw only when row is in drawing area */
				if (yPos >= ClipRect.y0) {
					GUI_DrawHLine(yPos, ClipRect.x0, ClipRect.x1);
				}
			}
			xPos = EffectSize - this->ScrollStateH.v;
			for (i = 0; i < NumColumns; i++) {
				xPos += pHeader->GetItemWidth(i);
				/* Break when all other columns are outside the drawing area */
				if (xPos > ClipRect.x1) {
					break;
				}
				/* Make sure that we draw only when column is in drawing area */
				if (xPos >= ClipRect.x0) {
					GUI_DrawVLine(xPos, ClipRect.y0, ClipRect.y1);
				}
			}
		}
		/* Draw the effect */
		WIDGET__EFFECT_DrawDown(this);
	}
	void _InvalidateRowAndBelow(int Sel) {
		if (Sel >= 0) {
			GUI_RECT Rect;
			int HeaderHeight, RowDistY;
			HeaderHeight = pHeader->GetHeight();
			RowDistY = _GetRowDistY();
			WM_GetInsideRectExScrollbar(this, &Rect);
			Rect.y0 += HeaderHeight + (Sel - this->ScrollStateV.v) * RowDistY;
			WM_InvalidateRect(this, &Rect);
		}
	}
	void _InvalidateInsideArea() {
		GUI_RECT Rect;
		int HeaderHeight;
		HeaderHeight = pHeader->GetHeight();
		WM_GetInsideRectExScrollbar(this, &Rect);
		Rect.y0 += HeaderHeight;
		WM_InvalidateRect(this, &Rect);
	}
	void _InvalidateRow(int Sel) {
		if (Sel >= 0) {
			GUI_RECT Rect;
			int HeaderHeight, RowDistY;
			HeaderHeight = pHeader->GetHeight();
			RowDistY = _GetRowDistY();
			WM_GetInsideRectExScrollbar(this, &Rect);
			Rect.y0 += HeaderHeight + (Sel - this->ScrollStateV.v) * RowDistY;
			Rect.y1 = Rect.y0 + RowDistY - 1;
			WM_InvalidateRect(this, &Rect);
		}
	}
	void _SetSelFromPos(const GUI_PID_STATE *pState) {
		GUI_RECT Rect;
		int x, y, HeaderHeight;
		HeaderHeight = pHeader->GetHeight();
		WM_GetInsideRectExScrollbar(this, &Rect);
		x = pState->x - Rect.x0;
		y = pState->y - Rect.y0 - HeaderHeight;
		Rect.x1 -= Rect.x0;
		Rect.y1 -= Rect.y0;
		if ((x >= 0) && (x <= Rect.x1) && (y >= 0) && (y <= (Rect.y1 - HeaderHeight))) {
			unsigned Sel;
			Sel = (y / _GetRowDistY()) + this->ScrollStateV.v;
			if (Sel < GUI_ARRAY_GetNumItems(&this->RowArray)) {
				SetSel(Sel);
			}
		}
	}
	void _OnTouch(const GUI_PID_STATE *pState) {
		int Notification;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				_SetSelFromPos(pState);
				Notification = WM_NOTIFICATION_CLICKED;
				WM_SetFocus(this);
			}
			else
				Notification = WM_NOTIFICATION_RELEASED;
		}
		else
			Notification = WM_NOTIFICATION_MOVED_OUT;
		_NotifyOwner(Notification);
	}
	int _OnKey(const WM_KEY_INFO *pInfo) {
		if (pInfo->PressedCnt > 0)
			switch (pInfo->Key) {
				case GUI_KEY_DOWN:
					IncSel();
					return 1;
				case GUI_KEY_UP:
					DecSel();
					return 1;
			}
		return 0;
	}
	int _GetXSize() {
		GUI_RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		return Rect.x1 + 1;
	}
	int _GetHeaderWidth() {
		int NumItems, i, r = 1;
		NumItems = pHeader->GetNumItems();
		if (NumItems) {
			for (i = 0, r = 0; i < NumItems; i++) {
				r += pHeader->GetItemWidth(i);
			}
		}
		if (this->ScrollStateH.v > (r - this->ScrollStateH.PageSize)) {
			r += this->ScrollStateH.PageSize - (r - this->ScrollStateH.v);
		}
		return r;
	}
	int _UpdateScrollPos() {
		int PrevScrollStateV;
		PrevScrollStateV = this->ScrollStateV.v;
		if (this->Sel >= 0) {
			WM_CheckScrollPos(&this->ScrollStateV, this->Sel, 0, 0);
		}
		else {
			WM_CheckScrollBounds(&this->ScrollStateV);
		}
		WM_CheckScrollBounds(&this->ScrollStateH);
		WIDGET__SetScrollState(this, &this->ScrollStateV, &this->ScrollStateH);
		return this->ScrollStateV.v - PrevScrollStateV;
	}
	int _UpdateScrollParas() {
		int NumRows;
		NumRows = GUI_ARRAY_GetNumItems(&this->RowArray);
		/* update vertical scrollbar */
		this->ScrollStateV.PageSize = _GetNumVisibleRows();
		this->ScrollStateV.NumItems = (NumRows) ? NumRows : 1;
		/* update horizontal scrollbar */
		this->ScrollStateH.PageSize = _GetXSize();
		this->ScrollStateH.NumItems = _GetHeaderWidth();
		return _UpdateScrollPos();
	}
	void _FreeAttached() {
		int i, j, NumRows, NumColumns;
		NumRows = GUI_ARRAY_GetNumItems(&this->RowArray);
		NumColumns = GUI_ARRAY_GetNumItems(&this->AlignArray);
		for (i = 0; i < NumRows; i++) {
			GUI_ARRAY *pRow;
			pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&this->RowArray, i);
			/* Delete attached info items */
			for (j = 0; j < NumColumns; j++) {
				Item *pItem;
				pItem = (Item *)GUI_ARRAY_GetpItem(pRow, j);
				if (pItem->hItemInfo) {
					GUI_ALLOC_Free(pItem->hItemInfo);
				}
			}
			/* Delete row */
			GUI_ARRAY_Delete(pRow);
		}
		GUI_ARRAY_Delete(&this->AlignArray);
		GUI_ARRAY_Delete(&this->RowArray);
	}
	ItemInfo *_GetpItemInfo(unsigned Column, unsigned Row, unsigned int Index) {
		ItemInfo *pItemInfo = 0;
		Item *pItem;
		if ((Column < GetNumColumns()) && (Row < GetNumRows()) && (Index < GUI_COUNTOF(pItemInfo->aTextColor))) {
			pItem = (Item *)GUI_ARRAY_GetpItem((GUI_ARRAY *)GUI_ARRAY_GetpItem(&RowArray, Row), Column);
			if (!pItem->hItemInfo) {
				int i;
				pItem->hItemInfo = GUI_ALLOC_AllocZero(sizeof(ItemInfo));
				pItemInfo = (ItemInfo *)(pItem->hItemInfo);
				for (i = 0; i < GUI_COUNTOF(pItemInfo->aTextColor); i++) {
					pItemInfo->aTextColor[i] = GetTextColor(i);
					pItemInfo->aBkColor[i] = GetBkColor(i);
				}
			}
			else {
				pItemInfo = (ItemInfo *)(pItem->hItemInfo);
			}
		}

		return pItemInfo;
	}

	static WM_PARAM _Callback(WM_Obj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (LISTVIEW_Obj *)hWin;
		/* Let widget handle the standard messages */
		if (!WIDGET_HandleActive(pObj, MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_NOTIFY_CLIENTCHANGE:
			case WM_SIZE:
				pObj->_UpdateScrollParas();
				return 0;
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const WM_NOTIFY_INFO *)Data;
				auto pWinSrc = pInfo->pWinSrc;
				switch (pInfo->Notification) {
					case WM_NOTIFICATION_CHILD_DELETED:
						/* make sure we do not send any messages to the header child once it has been deleted */
						if (pWinSrc == pObj->pHeader)
							pObj->pHeader = nullptr;
						break;
					case WM_NOTIFICATION_VALUE_CHANGED: {
						WM_SCROLL_STATE ScrollState;
						if (pWinSrc == WM_GetScrollbarV(pObj)) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateV.v = ScrollState.v;
							pObj->_InvalidateInsideArea();
							pObj->_NotifyOwner(WM_NOTIFICATION_SCROLL_CHANGED);
						}
						else if (pWinSrc == WM_GetScrollbarH(pObj)) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateH.v = ScrollState.v;
							pObj->_UpdateScrollParas();
							pObj->pHeader->SetScrollPos(pObj->ScrollStateH.v);
							pObj->_NotifyOwner(WM_NOTIFICATION_SCROLL_CHANGED);
						}
						break;
					}
					case WM_NOTIFICATION_SCROLLBAR_ADDED:
						pObj->_UpdateScrollParas();
						break;
				}
				return 0;
			}
			case WM_PAINT:
				pObj->_OnPaint((const GUI_RECT *)Data);
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const GUI_PID_STATE *)Data);
				return 0;
			case WM_KEY:
				if (pObj->_OnKey((const WM_KEY_INFO *)Data))
					return 0;
				break;
			case WM_DELETE:
				pObj->_FreeAttached();
				return 0;
		}
		return WM_DefaultProc(hWin, MsgId, Data);
	}

public:
	void IncSel() {
		int Sel = GetSel();
		SetSel(Sel + 1);
	}
	void DecSel() {
		int Sel = GetSel();
		if (Sel) {
			SetSel(Sel - 1);
		}
	}
	void AddColumn(int Width, const char *s, int Align) {
		unsigned NumRows;
		pHeader->AddItem(Width, s, Align);   /* Modify header */
		GUI_ARRAY_AddItem(&AlignArray, &Align, sizeof(int));
		NumRows = GetNumRows();
		if (NumRows) {
			GUI_ARRAY *pRow;
			unsigned i;
			for (i = 0; i < NumRows; i++) {
				pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&RowArray, i);
				GUI_ARRAY_AddItem(pRow, nullptr, sizeof(Item) + 1);
			}
		}
		_UpdateScrollParas();
		_InvalidateInsideArea();
	}
	void AddRow(const char **ppText) {
		int NumRows;
		NumRows = GUI_ARRAY_GetNumItems(&RowArray);
		/* Create GUI_ARRAY for the new row */
		if (GUI_ARRAY_AddItem(&RowArray, nullptr, sizeof(GUI_ARRAY)) == 0) {
			int i, NumColumns, NumBytes;
			GUI_ARRAY *pRow;
			const char *s;
			/* Add columns for the new row */
			NumColumns = pHeader->GetNumItems();
			for (i = 0; i < NumColumns; i++) {
				Item *pItem;
				pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&RowArray, NumRows);
				s = (ppText) ? *ppText++ : 0;
				if (s == 0) {
					ppText = 0;
				}
				NumBytes = GUI__strlen(s) + 1;     /* 0 if no string is specified (s == nullptr) */
				GUI_ARRAY_AddItem(pRow, nullptr, sizeof(Item) + NumBytes);
				pItem = (Item *)GUI_ARRAY_GetpItem(pRow, i);
				if (NumBytes > 1) {
					GUI__strcpy(pItem->acText, s);
				}
			}
			_UpdateScrollParas();
			_InvalidateRow(NumRows);
		}
	}
	void DeleteColumn(unsigned Index) {
		if (Index < GUI_ARRAY_GetNumItems(&AlignArray)) {
			unsigned NumRows, i;
			GUI_ARRAY *pRow;
			pHeader->DeleteItem(Index);
			GUI_ARRAY_DeleteItem(&AlignArray, Index);
			NumRows = GUI_ARRAY_GetNumItems(&RowArray);
			for (i = 0; i < NumRows; i++) {
				Item *pItem;
				pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&RowArray, i);
				/* Delete attached info items */
				pItem = (Item *)GUI_ARRAY_GetpItem(pRow, Index);
				if (pItem->hItemInfo) {
					GUI_ALLOC_Free(pItem->hItemInfo);
				}
				/* Delete cell */
				GUI_ARRAY_DeleteItem(pRow, Index);
			}
			_UpdateScrollParas();
			_InvalidateInsideArea();
		}
	}
	void DeleteRow(unsigned Index) {
		unsigned NumRows = GUI_ARRAY_GetNumItems(&RowArray);
		if (Index < NumRows) {
			auto pRow = (GUI_ARRAY *)GUI_ARRAY_GetpItem(&RowArray, Index);
			/* Delete attached info items */
			for (int i = 0, NumColumns = GUI_ARRAY_GetNumItems(pRow); i < NumColumns; i++) {
				Item *pItem;
				pItem = (Item *)GUI_ARRAY_GetpItem(pRow, i);
				if (pItem->hItemInfo) {
					GUI_ALLOC_Free(pItem->hItemInfo);
				}
			}
			/* Delete row */
			GUI_ARRAY_Delete(pRow);
			GUI_ARRAY_DeleteItem(&RowArray, Index);
			/* Adjust properties */
			if (Sel == (signed int)Index)
				Sel = -1;
			if (Sel > (signed int)Index)
				Sel--;
			if (_UpdateScrollParas())
				_InvalidateInsideArea();
			else
				_InvalidateRowAndBelow(Index);
		}
	}
	RGBC GetBkColor(unsigned Index) {
		RGBC Color = RGB_INVALID_COLOR;
		if (Index <= GUI_COUNTOF(Props.aBkColor)) {
			Color = Props.aBkColor[Index];
		}
		return Color;
	}
	PCFONT GetFont() {
		return Props.pFont;
	}
	HEADER_Obj *GetHeader() {
		return pHeader;
	}
	unsigned GetNumColumns() {
		return GUI_ARRAY_GetNumItems(&AlignArray);
	}
	unsigned GetNumRows() {
		return GUI_ARRAY_GetNumItems(&RowArray);
	}
	int GetSel() {
		return Sel;
	}
	RGBC GetTextColor(unsigned Index) {
		RGBC Color = RGB_INVALID_COLOR;
		if (Index <= GUI_COUNTOF(Props.aTextColor)) {
			Color = Props.aTextColor[Index];
		}
		return Color;
	}
	void SetBkColor(unsigned int Index, RGBC Color) {
		if (Index < GUI_COUNTOF(Props.aBkColor)) {
			if (Color != Props.aBkColor[Index]) {
				Props.aBkColor[Index] = Color;
				_InvalidateInsideArea();
			}
		}
	}
	void SetColumnWidth(unsigned int Index, int Width) {
		pHeader->SetItemWidth(Index, Width);
	}
	void SetFont(PCFONT pFont) {
		if (pFont != Props.pFont) {
			Props.pFont = pFont;
			_UpdateScrollParas();
			_InvalidateInsideArea();
		}
	}
	int SetGridVis(bool Show) {
		if (Show != ShowGrid) {
			ShowGrid = Show;
			_UpdateScrollParas();
			_InvalidateInsideArea();
		}
		return ShowGrid;
	}
	void SetItemTextColor(unsigned Column, unsigned Row, unsigned int Index, RGBC Color) {
		ItemInfo *pItemInfo;
		pItemInfo = _GetpItemInfo(Column, Row, Index);
		if (pItemInfo) {
			pItemInfo->aTextColor[Index] = Color;
		}
	}
	void SetItemBkColor(unsigned Column, unsigned Row, unsigned int Index, RGBC Color) {
		ItemInfo *pItemInfo;

		pItemInfo = _GetpItemInfo(Column, Row, Index);
		if (pItemInfo) {
			pItemInfo->aBkColor[Index] = Color;
		}
	}
	void SetItemText(unsigned Column, unsigned Row, const char *s) {
		if ((Column < GetNumColumns()) && (Row < GetNumRows())) {
			auto NumBytes = GUI__strlen(s) + 1;
			auto pItem = (Item *)GUI_ARRAY_ResizeItem((GUI_ARRAY *)GUI_ARRAY_GetpItem(&RowArray, Row), Column, sizeof(Item) + NumBytes);
			if (NumBytes > 1) {
				GUI__strcpy(pItem->acText, s);
			}
			_InvalidateRow(Row);
		}
	}
	void SetLBorder(unsigned BorderSize) {
		if (LBorder != BorderSize) {
			LBorder = BorderSize;
			_InvalidateInsideArea();
		}
	}
	void SetRBorder(unsigned BorderSize) {
		if (RBorder != BorderSize) {
			RBorder = BorderSize;
			_InvalidateInsideArea();
		}
	}
	unsigned SetRowHeight(unsigned RowHeight) {
		if (RowDistY != RowHeight) {
			RowDistY = RowHeight;
			_UpdateScrollParas();
			_InvalidateInsideArea();
		}
		return RowDistY;
	}
	void SetSel(int NewSel) {
		int MaxSel = GUI_ARRAY_GetNumItems(&RowArray) - 1;
		if (NewSel > MaxSel) {
			NewSel = MaxSel;
		}
		if (NewSel < 0) {
			NewSel = -1;
		}
		if (NewSel != Sel) {
			int OldSel = Sel;
			Sel = NewSel;
			if (_UpdateScrollPos()) {
				_InvalidateInsideArea();
			}
			else {
				_InvalidateRow(OldSel);
				_InvalidateRow(NewSel);
			}
			WM_NotifyParent(this, WM_NOTIFICATION_SEL_CHANGED);
		}
	}
	void SetTextAlign(unsigned int Index, int Align) {
		if (Index < GUI_ARRAY_GetNumItems(&AlignArray)) {
			int *pAlign = (int *)GUI_ARRAY_GetpItem(&AlignArray, Index);
			if (Align != *pAlign) {
				*pAlign = Align;
				_InvalidateInsideArea();
			}
		}
	}
	void SetTextColor(unsigned int Index, RGBC Color) {
		if (Index < GUI_COUNTOF(Props.aTextColor)) {
			if (Color != Props.aTextColor[Index]) {
				Props.aTextColor[Index] = Color;
				_InvalidateInsideArea();
			}
		}
	}
};

LISTVIEW_Obj::Properties LISTVIEW_Obj::DefaultProps;

LISTVIEW_Obj *LISTVIEW_CreateEx(int x0, int y0, int xsize, int ysize, WM_Obj *hParent,
								  int WinFlags, int ExFlags, int Id) {
	GUI_USE_PARA(ExFlags);
	/* Create the window */
	if ((xsize == 0) && (ysize == 0) && (x0 == 0) && (y0 == 0)) {
		GUI_RECT Rect = WM_GetClientRect(hParent);
		xsize = Rect.x1 - Rect.x0 + 1;
		ysize = Rect.y1 - Rect.y0 + 1;
	}
	auto pObj = (LISTVIEW_Obj *)WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, LISTVIEW_Obj::_Callback,
								  sizeof(LISTVIEW_Obj) - sizeof(WM_Obj));
	if (pObj) {
		/* Init widget specific variables */
		WIDGET__Init(pObj, Id, WIDGET_STATE_FOCUSSABLE);
		/* Init member variables */
		pObj->Props = LISTVIEW_Obj::DefaultProps;
		pObj->ShowGrid = 0;
		pObj->RowDistY = 0;
		pObj->Sel = -1;
		pObj->LBorder = 1;
		pObj->RBorder = 1;
		pObj->pHeader = HEADER_CreateEx(0, 0, 0, 0, pObj, WM_CF_SHOW, 0, 0);
		pObj->_UpdateScrollParas();
	}
	else {
	}
	return pObj;
}
WM_Obj *LISTVIEW_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_Obj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	return LISTVIEW_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
							  hWinParent, 0, pCreateInfo->Flags, pCreateInfo->Id);
}

}
