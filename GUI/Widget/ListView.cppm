module;

#include "GUI.h"

export module TUX.Widget.ListView;

import TUX.Widget;
import TUX.Widget.Header;
import TUX.Widget.ScrollBar;

import TUX.Array;

export {

constexpr uint16_t LISTVIEW_CF_SHOWGRID = WIDGET_STATE_USER<0>;
	
enum LISTVIEW_CI {
	 LISTVIEW_CI_UNSEL = 0,
	 LISTVIEW_CI_SEL,
	 LISTVIEW_CI_SELFOCUS
};

class ListView : public Widget {

public:
	struct Properties {
		PCFONT pFont{ GUI_DEFAULT_FONT };
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
		ItemInfo *pItemInfo;
		char *pText;
	};
	Header *pHeader;
	ARRAY<ARRAY<Item>> RowArray; /* One entry per line. Every entry is a ARRAY<Item> */
	ARRAY<TEXTALIGN>   AlignArray; /* One entry per column */
	int16_t     Sel = -1;
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
			Props.pFont->YSize + (States & LISTVIEW_CF_SHOWGRID ? 1 : 0);
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
		/* Init some values */
		auto NumColumns = pHeader->GetNumItems();
		auto NumRows = RowArray.NumItems();
		auto NumVisRows = _GetNumVisibleRows();
		auto RowDistY = _GetRowDistY();
		auto EffectSize = this->EffectSize();
		auto yPos = pHeader->GetHeight() + EffectSize;
		auto EndRow = ScrollStateV.v + (((NumVisRows + 1) > NumRows) ? NumRows : NumVisRows + 1);
		/* Calculate clipping rectangle */
		RECT rClient;
		auto rClip = *pClipRect - GetOrg();
		WM_GetInsideRectExScrollbar(this, &rClient);
		rClip &= rClient;
		/* Set drawing color, font and text mode */
		GUI.Color(Props.aTextColor[0]);
		GUI.Font(Props.pFont);
		/* Do the drawing */
		for (auto i = ScrollStateV.v; i < EndRow; i++) {
			auto &pRow = RowArray[i];
			rClient.y0 = yPos;
			/* Break when all other rows are outside the drawing area */
			if (rClient.y0 > rClip.y1)
				break;
			rClient.y1 = yPos + RowDistY - 1;
			/* Make sure that we draw only when row is in drawing area */
			if (rClient.y1 >= rClip.y0) {
				auto ColorIndex =
						i == Sel ?
						States & WIDGET_STATE_FOCUS ? LISTVIEW_CI_SELFOCUS : LISTVIEW_CI_SEL :
						LISTVIEW_CI_UNSEL;
				GUI.BkColor(Props.aBkColor[ColorIndex]);
				/* Iterate over all columns */
				if (States & LISTVIEW_CF_SHOWGRID)
					rClient.y1--;
				auto xPos = EffectSize - this->ScrollStateH.v;
				for (auto j = 0; j < NumColumns; j++) {
					auto Width = pHeader->GetItemWidth(j);
					rClient.x0 = xPos;
					/* Break when all other columns are outside the drawing area */
					if (rClient.x0 > rClip.x1)
						break;
					rClient.x1 = xPos + Width - 1;
					/* Make sure that we draw only when column is in drawing area */
					if (rClient.x1 >= rClip.x0) {
						auto &item = pRow[j];
						if (auto pItemInfo = item.pItemInfo) {
							GUI.BkColor(pItemInfo->aBkColor[ColorIndex]);
							GUI.Color(pItemInfo->aTextColor[ColorIndex]);
						}
						else {
							GUI.Color(Props.aTextColor[ColorIndex]);
						}
						/* Clear background */
						GUI_ClearRect(rClient);
						/* Draw text */
						rClient.x0 += LBorder;
						rClient.x1 -= RBorder;
						auto Align = AlignArray[j];
						GUI_DispStringInRect(item.pText, rClient, Align);
						if (auto pItemInfo = item.pItemInfo)
							GUI.BkColor(pItemInfo->aBkColor[ColorIndex]);
					}
					xPos += Width;
				}
				/* Clear unused area to the right of items */
				if (xPos <= rClip.x1)
					GUI_ClearRect({ xPos, rClient.y0, rClip.x1, rClient.y1 });
			}
			yPos += RowDistY;
		}
		/* Clear unused area below items */
		if (yPos <= rClip.y1) {
			GUI.BkColor(Props.aBkColor[0]);
			GUI_ClearRect({ rClip.x0, yPos, rClip.x1, rClip.y1 });
		}
		/* Draw grid */
		if (States & LISTVIEW_CF_SHOWGRID) {
			GUI.Color(Props.GridColor);
			yPos = pHeader->GetHeight() + EffectSize - 1;
			for (auto i = 0; i < NumVisRows; i++) {
				yPos += RowDistY;
				if (rClip.y0 <= yPos && yPos <= rClip.y1)
					GUI_DrawHLine(yPos, rClip.x0, rClip.x1);
			}
			auto xPos = EffectSize - this->ScrollStateH.v;
			for (auto i = 0; i < NumColumns; i++) {
				xPos += pHeader->GetItemWidth(i);
				if (rClip.x0 <= xPos && xPos <= rClip.x1)
					GUI_DrawVLine(xPos, rClip.y0, rClip.y1);
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
			Rect.y0 += HeaderHeight + (Sel - ScrollStateV.v) * RowDistY;
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
			Rect.y0 += HeaderHeight + (Sel - ScrollStateV.v) * RowDistY;
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
			auto Sel = (y / _GetRowDistY()) + ScrollStateV.v;
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
		ScrollState(ScrollStateV, ScrollStateH);
		return ScrollStateV.v - PrevScrollStateV;
	}
	int _UpdateScrollParas() {
		int NumRows;
		NumRows = RowArray.NumItems();
		/* update vertical scrollbar */
		ScrollStateV.PageSize = _GetNumVisibleRows();
		ScrollStateV.NumItems = (NumRows) ? NumRows : 1;
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
				auto &pRow = RowArray[i];
				/* Delete attached info items */
				for (j = 0; j < NumColumns; j++) {
					auto &item = pRow[j];
					GUI_ALLOC_FreePtr((void **)&item.pText);
					if (item.pItemInfo) {
						GUI_ALLOC_Free(item.pItemInfo);
					}
				}
				/* Delete row */
				pRow.Delete();
			}
		this->AlignArray.Delete();
		RowArray.Delete();
	}

	ItemInfo *_GetpItemInfo(uint16_t Column, uint16_t Row, LISTVIEW_CI Index) {
		if (Index >= GUI_COUNTOF(ItemInfo::aTextColor))
			return nullptr;
		if (Column >= GetNumColumns() || Row >= GetNumRows())
			return nullptr;
		auto pItem = &RowArray[Row][Column];
		auto pItemInfo = pItem->pItemInfo;
		if (pItemInfo)
			return pItemInfo;
		pItemInfo = pItem->pItemInfo = (ItemInfo *)GUI_ALLOC_Alloc(sizeof(ItemInfo));
		pItemInfo->aTextColor[0] = Props.aTextColor[0];
		pItemInfo->aTextColor[1] = Props.aTextColor[1];
		pItemInfo->aBkColor[0] = Props.aBkColor[0];
		pItemInfo->aBkColor[1] = Props.aBkColor[1];
		return pItemInfo;
	}

	static WM_PARAM _Callback(WObj *pWin, int MsgId, WM_PARAM Data) {
		auto pObj = (ListView *)pWin;
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
						if (pWinSrc == pObj->GetScrollbarV()) {
							pObj->ScrollStateV.v = pWinSrc->ScrollState().v;
							pObj->_InvalidateInsideArea();
						}
						else if (pWinSrc == pObj->GetScrollbarH()) {
							pObj->ScrollStateH.v = pWinSrc->ScrollState().v;
							pObj->pHeader->SetScrollPos(pObj->ScrollStateH.v);
							pObj->_UpdateScrollParas();
						}
						else break;
						pObj->_NotifyOwner(WM_NOTIFICATION_SCROLL_CHANGED);
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
		return DefaultProc(pWin, MsgId, Data);
	}

public:
	ListView(RECT r, WM_CF Style, WObj *pParent, uint16_t Id) :
		Widget(r, Style, _Callback, pParent, Id, WIDGET_STATE_FOCUSSABLE) {
		pHeader = new Header(RECT{}, WC_VISIBLE, this, 0);
		_UpdateScrollParas();
	}
	static Widget *CreateIndirect(const CreateStruct *pCreateInfo, WObj *pWinParent, int x0, int y0, WM_CALLBACK *cb) {
		return new ListView(
			RECT::LeftTop({ pCreateInfo->x0 + x0, pCreateInfo->y0 + y0 },
						  { pCreateInfo->xSize, pCreateInfo->ySize }),
			pCreateInfo->Flags, pWinParent, pCreateInfo->Id);
	}

public:

#pragma region Properties

	UCFONT Font() const { return *Props.pFont; }
	void Font(PCFONT pFont) {
		if (Props.pFont == pFont)
			return;
		Props.pFont = pFont;
		_UpdateScrollParas();
		_InvalidateInsideArea();
	}

	RGBC BkColor(LISTVIEW_CI Index) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return RGB_INVALID;
		return Props.aBkColor[Index];
	}
	void BkColor(LISTVIEW_CI Index, RGBC Color) {
		if (Index >= GUI_COUNTOF(Props.aBkColor))
			return;
		if (Props.aBkColor[Index] == Color)
			return;
		Props.aBkColor[Index] = Color;
		_InvalidateInsideArea();
	}

	RGBC TextColor(LISTVIEW_CI Index) {
		if (Index >= GUI_COUNTOF(Props.aTextColor))
			return RGB_INVALID;
		return Props.aTextColor[Index];
	}
	void TextColor(LISTVIEW_CI Index, RGBC Color) {
		if (Index >= GUI_COUNTOF(Props.aTextColor))
			return;
		if (Props.aTextColor[Index] == Color)
			return;
		Props.aTextColor[Index] = Color;
		_InvalidateInsideArea();
	}

	void TextAlign(LISTVIEW_CI Index, TEXTALIGN Align) {
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

	void SetGridVis(bool bShow) {
		auto NewStates = bShow ?
			States | LISTVIEW_CF_SHOWGRID :
			States & ~LISTVIEW_CF_SHOWGRID;
		if (States == NewStates)
			return;
		States = NewStates;
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
			GUI__SetText(row[i].pText, s);
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
			GUI_ALLOC_FreePtr((void **)&item.pItemInfo);
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
			auto &item = Row[i];
			GUI_ALLOC_FreePtr((void **)&item.pText);
			GUI_ALLOC_FreePtr((void **)&item.pItemInfo);
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
	
	void ItemTextColor(uint16_t Column, uint16_t Row, LISTVIEW_CI Index, RGBC Color) {
		if (auto pItemInfo = _GetpItemInfo(Column, Row, Index))
			pItemInfo->aTextColor[Index] = Color;
	}
	void ItemBkColor(uint16_t Column, uint16_t Row, LISTVIEW_CI Index, RGBC Color) {
		if (auto pItemInfo = _GetpItemInfo(Column, Row, Index))
			pItemInfo->aBkColor[Index] = Color;
	}
	void SetItemText(uint16_t Column, uint16_t Row, const char *s) {
		if (Column < GetNumColumns() && Row < GetNumRows()) {
			auto &item = RowArray[Row][Column];
			GUI__SetText(item.pText, s);
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
