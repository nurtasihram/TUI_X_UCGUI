module;

#include "GUI_Protected.h"

export module TUX.Widget.ListView;

import TUX.Widget;
import TUX.Widget.Header;
import TUX.Widget.ScrollBar;

import TUX.Array;

export {

enum LISTVIEW_CI {
	 LISTVIEW_CI_UNSEL = 0,
	 LISTVIEW_CI_SEL,
	 LISTVIEW_CI_SELFOCUS
};

class ListView : public Widget {

public:
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
	
private:
	Properties Props = DefaultProps;

	struct ItemInfo {
		RGBC aBkColor[3];
		RGBC aTextColor[3];
	};
	struct Item {
		WM_HMEM hItemInfo;
		char *pText;
	};
	Header *pHeader;
	ARRAY<ARRAY<Item>> RowArray; /* One entry per line. Every entry is a ARRAY<Item> */
	ARRAY<TEXTALIGN>   AlignArray; /* One entry per column */
	int16_t     Sel = -1;
	bool        ShowGrid = false;
	uint16_t    RowDistY = 0, LBorder = 1, RBorder = 1;
	WM_SCROLL_STATE ScrollStateV, ScrollStateH;
	WObj *pOwner = nullptr;

	void _NotifyOwner(int Notification) {
		auto pOwner = this->pOwner ? this->pOwner : Parent();
		NOTIFY_INFO Info;
		Info.Notification = Notification;
		Info.pWinSrc = this;
		pOwner->Require(WM_NOTIFY_PARENT, (WM_PARAM)&Info);
	}

	auto _GetRowDistY() {
		return RowDistY ? RowDistY :
			Props.pFont->DistY() + (ShowGrid ? 1 : 0);
	}
	auto _GetNumVisibleRows() {
		RECT Rect;
		WM_GetInsideRectExScrollbar(this, &Rect);
		if (auto RowDistY = _GetRowDistY()) {
			auto r = (Rect.YSize() - pHeader->GetHeight()) / RowDistY;
			return r ? r : 1;
		}
		return 1;
	}
	void _OnPaint(const RECT *pClipRect) {
		RECT ClipRect, Rect;
		int NumRows, NumVisRows, NumColumns;
		int LBorder, RBorder, EffectSize;
		int xPos, yPos, Width, RowDistY;
		int Align, i, j, EndRow;
		/* Init some values */
		NumColumns = pHeader->GetNumItems();
		NumRows = RowArray.NumItems();
		NumVisRows = _GetNumVisibleRows();
		RowDistY = _GetRowDistY();
		LBorder = this->LBorder;
		RBorder = this->RBorder;
		EffectSize = this->EffectSize();
		yPos = pHeader->GetHeight() + EffectSize;
		EndRow = this->ScrollStateV.v + (((NumVisRows + 1) > NumRows) ? NumRows : NumVisRows + 1);
		/* Calculate clipping rectangle */
		ClipRect = *pClipRect - this->Rect.LeftTop();
		WM_GetInsideRectExScrollbar(this, &Rect);
		ClipRect &= Rect;
		/* Set drawing color, font and text mode */
		GUI.SetColor(Props.aTextColor[0]);
		GUI.SetFont(Props.pFont);
		GUI.SetTextMode(DRAWMODE_TRANS);
		/* Do the drawing */
		for (i = this->ScrollStateV.v; i < EndRow; i++) {
			auto &pRow = this->RowArray[i];
			{
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
						ColorIndex = (GetStates() & WIDGET_STATE_FOCUS) ? 2 : 1;
					}
					else {
						ColorIndex = 0;
					}
					GUI.SetBkColor(Props.aBkColor[ColorIndex]);
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
							auto &pItem = pRow[j];
							if (pItem.hItemInfo) {
								ItemInfo *pItemInfo;
								pItemInfo = (ItemInfo *)(pItem.hItemInfo);
								GUI.SetBkColor(pItemInfo->aBkColor[ColorIndex]);
								GUI.SetColor(pItemInfo->aTextColor[ColorIndex]);
							}
							else {
								GUI.SetColor(Props.aTextColor[ColorIndex]);
							}
							/* Clear background */
							GUI_ClearRect(Rect);
							/* Draw text */
							Rect.x0 += LBorder;
							Rect.x1 -= RBorder;
							Align = this->AlignArray[j];
							GUI_DispStringInRect(pItem.pText, &Rect, Align);
							if (pItem.hItemInfo)
								GUI.SetBkColor(Props.aBkColor[ColorIndex]);
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
			GUI.SetBkColor(Props.aBkColor[0]);
			GUI_ClearRect({ ClipRect.x0, yPos, ClipRect.x1, ClipRect.y1 });
		}
		/* Draw grid */
		if (this->ShowGrid) {
			GUI.SetColor(Props.GridColor);
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
		DrawDown();
	}
	void _InvalidateRowAndBelow(int Sel) {
		if (Sel >= 0) {
			RECT Rect;
			int HeaderHeight, RowDistY;
			HeaderHeight = pHeader->GetHeight();
			RowDistY = _GetRowDistY();
			WM_GetInsideRectExScrollbar(this, &Rect);
			Rect.y0 += HeaderHeight + (Sel - this->ScrollStateV.v) * RowDistY;
			Invalidate(&Rect);
		}
	}
	void _InvalidateInsideArea() {
		RECT Rect;
		int HeaderHeight;
		HeaderHeight = pHeader->GetHeight();
		WM_GetInsideRectExScrollbar(this, &Rect);
		Rect.y0 += HeaderHeight;
		Invalidate(&Rect);
	}
	void _InvalidateRow(int Sel) {
		if (Sel >= 0) {
			RECT Rect;
			int HeaderHeight, RowDistY;
			HeaderHeight = pHeader->GetHeight();
			RowDistY = _GetRowDistY();
			WM_GetInsideRectExScrollbar(this, &Rect);
			Rect.y0 += HeaderHeight + (Sel - this->ScrollStateV.v) * RowDistY;
			Rect.y1 = Rect.y0 + RowDistY - 1;
			Invalidate(&Rect);
		}
	}
	void _SetSelFromPos(const PID_STATE *pState) {
		RECT Rect;
		int x, y, HeaderHeight;
		HeaderHeight = pHeader->GetHeight();
		WM_GetInsideRectExScrollbar(this, &Rect);
		x = pState->x - Rect.x0;
		y = pState->y - Rect.y0 - HeaderHeight;
		Rect.x1 -= Rect.x0;
		Rect.y1 -= Rect.y0;
		if ((x >= 0) && (x <= Rect.x1) && (y >= 0) && (y <= (Rect.y1 - HeaderHeight))) {
			auto Sel = (y / _GetRowDistY()) + this->ScrollStateV.v;
			if (Sel < RowArray.NumItems())
				SetSel(Sel);
		}
	}
	void _OnTouch(const PID_STATE *pState) {
		int Notification;
		if (pState) {  /* Something happened in our area (pressed or released) */
			if (pState->Pressed) {
				_SetSelFromPos(pState);
				Notification = WM_NOTIFICATION_CLICKED;
				SetFocus();
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
		RECT Rect;
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
		auto PrevScrollStateV = ScrollStateV.v;
		if (Sel >= 0)
			ScrollStateV.CheckPos(Sel, 0, 0);
		else
			ScrollStateV.Bounds();
		ScrollStateH.Bounds();
		SetScrollState(ScrollStateV, ScrollStateH);
		return ScrollStateV.v - PrevScrollStateV;
	}
	int _UpdateScrollParas() {
		int NumRows;
		NumRows = RowArray.NumItems();
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
		NumRows = RowArray.NumItems();
		NumColumns = AlignArray.NumItems();
		for (i = 0; i < NumRows; i++) {
				auto &pRow = this->RowArray[i];
				/* Delete attached info items */
				for (j = 0; j < NumColumns; j++) {
					auto &pItem = pRow[j];
					GUI_ALLOC_FreePtr((void **)&pItem.pText);
					if (pItem.hItemInfo) {
						GUI_ALLOC_Free(pItem.hItemInfo);
					}
				}
				/* Delete row */
				pRow.Delete();
			}
		this->AlignArray.Delete();
		this->RowArray.Delete();
	}

	ItemInfo *_GetpItemInfo(uint16_t Column, uint16_t Row, LISTVIEW_CI Index) {
		if (Index >= GUI_COUNTOF(ItemInfo::aTextColor))
			return nullptr;
		if (Column >= GetNumColumns() || Row >= GetNumRows())
			return nullptr;
		auto pItem = &RowArray[Row][Column];
		if (pItem->hItemInfo)
			return (ItemInfo *)(pItem->hItemInfo);
		pItem->hItemInfo = GUI_ALLOC_AllocZero(sizeof(ItemInfo));
		auto pItemInfo = (ItemInfo *)(pItem->hItemInfo);
		pItemInfo->aTextColor[0] = Props.aTextColor[0];
		pItemInfo->aTextColor[1] = Props.aTextColor[1];
		pItemInfo->aBkColor[0] = Props.aBkColor[0];
		pItemInfo->aBkColor[1] = Props.aBkColor[1];
		return pItemInfo;
	}

	static WM_PARAM _Callback(WObj *hWin, int MsgId, WM_PARAM Data) {
		auto pObj = (ListView *)hWin;
		/* Let widget handle the standard messages */
		if (!pObj->HandleActive(MsgId, &Data))
			return Data;
		switch (MsgId) {
			case WM_NOTIFY_CLIENTCHANGE:
			case WM_SIZE:
				pObj->_UpdateScrollParas();
				return 0;
			case WM_NOTIFY_PARENT: {
				auto pInfo = (const NOTIFY_INFO *)Data;
				auto pWinSrc = pInfo->pWinSrc;
				switch (pInfo->Notification) {
					case WM_NOTIFICATION_CHILD_DELETED:
						/* make sure we do not send any messages to the header child once it has been deleted */
						if (pWinSrc == pObj->pHeader)
							pObj->pHeader = nullptr;
						break;
					case WM_NOTIFICATION_VALUE_CHANGED: {
						WM_SCROLL_STATE ScrollState;
						if (pWinSrc == pObj->GetScrollbarV()) {
							WM_GetScrollState(pWinSrc, &ScrollState);
							pObj->ScrollStateV.v = ScrollState.v;
							pObj->_InvalidateInsideArea();
							pObj->_NotifyOwner(WM_NOTIFICATION_SCROLL_CHANGED);
						}
						else if (pWinSrc == pObj->GetScrollbarH()) {
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
				pObj->_OnPaint((const RECT *)Data);
				return 0;
			case WM_TOUCH:
				pObj->_OnTouch((const PID_STATE *)Data);
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
	ListView(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget(r, Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE) {
		pHeader = new Header(RECT{}, WC_VISIBLE, this, 0);
		_UpdateScrollParas();
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *hWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new ListView(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, hWinParent, pCreateInfo->Id);
	}

public:

#pragma region Properties

	PCFONT GetFont() { return Props.pFont; }
	void SetFont(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		_UpdateScrollParas();
		_InvalidateInsideArea();
	}

	RGBC GetBkColor(LISTVIEW_CI Index) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return RGB_INVALID_COLOR;
		return Props.aBkColor[Index];
	}
	void SetBkColor(LISTVIEW_CI Index, RGBC Color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		if (Props.aBkColor[Index] == Color)
			return;
		Props.aBkColor[Index] = Color;
		_InvalidateInsideArea();
	}

	RGBC GetTextColor(LISTVIEW_CI Index) {
		if (Index >= GUI_COUNTOF(Props.aTextColor))
			return RGB_INVALID_COLOR;
		return Props.aTextColor[Index];
	}
	void SetTextColor(LISTVIEW_CI Index, RGBC Color) {
		if (Index >= GUI_COUNTOF(Props.aTextColor))
			return;
		if (Props.aTextColor[Index] == Color)
			return;
		Props.aTextColor[Index] = Color;
		_InvalidateInsideArea();
	}

	void SetTextAlign(LISTVIEW_CI Index, TEXTALIGN Align) {
		if (Index >= AlignArray.NumItems())
			return;
		if (AlignArray[Index] == Align)
			return;
		AlignArray[Index] = Align;
		_InvalidateInsideArea();
	}

	void SetLBorder(uint16_t BorderSize) {
		if (LBorder == BorderSize)
			return;
		LBorder = BorderSize;
		_InvalidateInsideArea();
	}
	void SetRBorder(uint16_t BorderSize) {
		if (RBorder == BorderSize)
			return;
		RBorder = BorderSize;
		_InvalidateInsideArea();
	}

	void SetRowHeight(uint16_t RowHeight) {
		if (RowDistY == RowHeight)
			return;
		RowDistY = RowHeight;
		_UpdateScrollParas();
		_InvalidateInsideArea();
	}

	void SetGridVis(bool Show) {
		if (ShowGrid == Show)
			return;
		ShowGrid = Show;
		_UpdateScrollParas();
		_InvalidateInsideArea();
	}

#pragma endregion

	void AddColumn(int Width, const char *s, TEXTALIGN Align) {
		pHeader->AddItem(Width, s, Align); /* Modify header */
		AlignArray.AddItem(&Align);
		for (int i = 0, NumRows = GetNumRows(); i < NumRows; i++)
			RowArray[i].AddItem();
		_UpdateScrollParas();
		_InvalidateInsideArea();
	}
	void AddRow(const char **ppText) {
		auto NumRows = RowArray.NumItems();
		/* Create ARRAY<Item> for the new row */
		if (RowArray.AddItem())
			return;
		/* Add columns for the new row */
		for (int i = 0, NumColumns = pHeader->GetNumItems(); i < NumColumns; i++) {
			auto s = ppText ? *ppText++ : 0;
			if (s == 0)
				ppText = 0;
			auto &row = RowArray[NumRows];
			row.AddItem();
			GUI__SetText(&row[i].pText, s);
		}
		_UpdateScrollParas();
		_InvalidateRow(NumRows);
	}

	void DeleteColumn(unsigned Index) {
		if (Index >= AlignArray.NumItems())
			return;
		pHeader->DeleteItem(Index);
		AlignArray.DeleteItem(Index);
		for (int i = 0, NumRows = RowArray.NumItems(); i < NumRows; i++) {
			auto &Row = RowArray[i];
			/* Delete attached info items */
			auto &item = Row[Index];
			GUI_ALLOC_FreePtr((void **)&item.pText);
			GUI_ALLOC_Free(item.hItemInfo);
			/* Delete cell */
			Row.DeleteItem(Index);
		}
		_UpdateScrollParas();
		_InvalidateInsideArea();
	}

	void DeleteRow(int16_t Index) {
		if (Index >= RowArray.NumItems())
			return;
		auto &Row = RowArray[Index];
		/* Delete attached info items */
		for (int i = 0, NumColumns = Row.NumItems(); i < NumColumns; i++) {
			auto &pItem = Row[i];
			GUI_ALLOC_FreePtr((void **)&pItem.pText);
			GUI_ALLOC_Free(pItem.hItemInfo);
		}
		/* Delete row */
		Row.Delete();
		RowArray.DeleteItem(Index);
		/* Adjust properties */
		if (Sel == Index)
			Sel = -1;
		if (Sel > Index)
			Sel--;
		if (_UpdateScrollParas())
			_InvalidateInsideArea();
		else
			_InvalidateRowAndBelow(Index);
	}
	
	void SetItemTextColor(uint16_t Column, uint16_t Row, LISTVIEW_CI Index, RGBC Color) {
		if (auto pItemInfo = _GetpItemInfo(Column, Row, Index))
			pItemInfo->aTextColor[Index] = Color;
	}
	void SetItemBkColor(uint16_t Column, uint16_t Row, LISTVIEW_CI Index, RGBC Color) {
		if (auto pItemInfo = _GetpItemInfo(Column, Row, Index))
			pItemInfo->aBkColor[Index] = Color;
	}
	void SetItemText(uint16_t Column, uint16_t Row, const char *s) {
		if (Column < GetNumColumns() && Row < GetNumRows()) {
			auto &item = RowArray[Row][Column];
			GUI__SetText(&item.pText, s);
			_InvalidateRow(Row);
		}
	}

	Header *GetHeader() { return pHeader; }
	void SetColumnWidth(unsigned int Index, int Width)
	{ pHeader->SetItemWidth(Index, Width); }

	auto GetNumColumns() { return AlignArray.NumItems(); }
	auto GetNumRows() { return RowArray.NumItems(); }

	auto GetSel() { return Sel; }
	void SetSel(int NewSel) {
		int MaxSel = RowArray.NumItems() - 1;
		if (NewSel > MaxSel)
			NewSel = MaxSel;
		if (NewSel < 0)
			NewSel = -1;
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
			NotifyParent(WM_NOTIFICATION_SEL_CHANGED);
		}
	}
	void IncSel() { SetSel(GetSel() + 1); }
	void DecSel() {
		if (auto Sel = GetSel())
			SetSel(Sel - 1);
	}
};

ListView::Properties ListView::DefaultProps;

}
