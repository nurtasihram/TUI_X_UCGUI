#pragma once

#include "LISTBOX.h"
#include "WM.h"
#include "GUI_ARRAY.h"
#include "WIDGET.h"

#define LISTBOX_ITEM_SELECTED (1 << 0)
#define LISTBOX_ITEM_DISABLED (1 << 1)

typedef struct {
	uint16_t xSize, ySize;
	uint8_t Status;
	char acText[1];
} LISTBOX_ITEM;

typedef struct {
	const GUI_FONT *pFont;
	uint16_t ScrollStepH;
	RGB_COLOR aBackColor[4];
	RGB_COLOR aTextColor[4];
} LISTBOX_PROPS;

typedef struct {
	WIDGET Widget;
	GUI_ARRAY ItemArray;
	WIDGET_DRAW_ITEM_FUNC *pfDrawItem;
	WM_SCROLL_STATE ScrollStateV;
	WM_SCROLL_STATE ScrollStateH;
	LISTBOX_PROPS Props;
	WM_HWIN hOwner;
	int16_t Sel;                        /* current selection */
	uint8_t Flags;
	uint8_t  ScrollbarWidth;
	uint16_t ItemSpacing;
} LISTBOX_Obj;

extern LISTBOX_PROPS LISTBOX_DefaultProps;
unsigned LISTBOX__GetNumItems(LISTBOX_Obj *pObj);
const char *LISTBOX__GetpString(LISTBOX_Obj *pObj, int Index);
void LISTBOX__InvalidateInsideArea(LISTBOX_Handle hObj);
void LISTBOX__InvalidateItem(LISTBOX_Obj *pObj, int Sel);
void LISTBOX__InvalidateItemAndBelow(LISTBOX_Obj *pObj, int Sel);
void LISTBOX__InvalidateItemSize(LISTBOX_Obj *pObj, unsigned Index);
void LISTBOX__SetScrollbarWidth(LISTBOX_Obj *pObj);
