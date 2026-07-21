#pragma once

#include "LISTBOX.h"
#include "WM.h"
#include "GUI_ARRAY.h"
#include "WIDGET.h"

#define LISTBOX_ITEM_SELECTED (1 << 0)
#define LISTBOX_ITEM_DISABLED (1 << 1)

struct LISTBOX_ITEM {
	uint16_t xSize, ySize;
	uint8_t Status;
	char acText[1];
};

struct LISTBOX_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC aBkColor[4]{
			/* Unselect */			RGB_WHITE,
			/* Selected */			RGB_GRAY,
			/* Selected focussed */	RGB_DARKBLUE,
			/* Disabled */			RGB_GRAYL(0xC0)
		};
		RGBC aTextColor[4]{
			/* Unselect */			RGB_BLACK,
			/* Selected */			RGB_WHITE,
			/* Selected focussed */	RGB_WHITE,
			/* Disabled */			RGB_GRAY
		};
		uint8_t ScrollStepH{ 10 };
	} static DefaultProps;
	Properties Props;
	GUI_ARRAY ItemArray;
	WIDGET_DRAW_ITEM_FUNC *pfDrawItem;
	WM_SCROLL_STATE ScrollStateV, ScrollStateH;
	WM_HWIN hOwner;
	int16_t Sel; /* current selection */
	uint8_t Flags;
	uint8_t  ScrollbarWidth;
	uint16_t ItemSpacing;
};

unsigned LISTBOX__GetNumItems(LISTBOX_Obj *pObj);
const char *LISTBOX__GetpString(LISTBOX_Obj *pObj, int Index);
void LISTBOX__InvalidateInsideArea(LISTBOX_Handle hObj);
void LISTBOX__InvalidateItem(LISTBOX_Obj *pObj, int Sel);
void LISTBOX__InvalidateItemAndBelow(LISTBOX_Obj *pObj, int Sel);
void LISTBOX__InvalidateItemSize(LISTBOX_Obj *pObj, unsigned Index);
void LISTBOX__SetScrollbarWidth(LISTBOX_Obj *pObj);
