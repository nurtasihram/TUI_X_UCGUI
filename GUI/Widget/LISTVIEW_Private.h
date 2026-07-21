#pragma once

#include "WM.h"
#include "LISTVIEW.h"
#include "GUI_ARRAY.h"

typedef struct {
	RGBC aBkColor[3];
	RGBC aTextColor[3];
} LISTVIEW_ITEM_INFO;

typedef struct {
	WM_HMEM hItemInfo;
	char acText[1];
} LISTVIEW_ITEM;

struct LISTVIEW_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC aBkColor[3]{
			RGB_WHITE,   /* Not selected */
			RGB_GRAY,    /* Selected, no focus */
			RGB_BLUE     /* Selected, focus */
		};
		RGBC aTextColor[3]{
			RGB_BLACK,   /* Not selected */
			RGB_WHITE,   /* Selected, no focus */
			RGB_WHITE    /* Selected, focus */
		};
		RGBC GridColor{ RGB_LIGHTGRAY };
	} static DefaultProps;
	Properties Props;
	HEADER_Handle   hHeader;
	GUI_ARRAY       RowArray;         /* One entry per line. Every entry is a handle of GUI_ARRAY of strings */
	GUI_ARRAY       AlignArray;       /* One entry per column */
	int16_t         Sel;
	int16_t         ShowGrid;
	uint16_t        RowDistY;
	uint16_t        LBorder;
	uint16_t        RBorder;
	WM_SCROLL_STATE ScrollStateV;
	WM_SCROLL_STATE ScrollStateH;
	WM_Obj *         hOwner;
};


int      LISTVIEW__UpdateScrollParas(LISTVIEW_Obj *pObj);
void     LISTVIEW__InvalidateInsideArea(LISTVIEW_Obj *pObj);
unsigned LISTVIEW__GetRowDistY(LISTVIEW_Obj *pObj);
void     LISTVIEW__InvalidateRow(LISTVIEW_Obj *pObj, int Sel);
int      LISTVIEW__UpdateScrollPos(LISTVIEW_Obj *pObj);
