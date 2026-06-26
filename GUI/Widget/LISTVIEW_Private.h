#pragma once

#include "WM.h"
#include "LISTVIEW.h"
#include "GUI_ARRAY.h"

typedef struct {
	RGB_COLOR aBkColor[3];
	RGB_COLOR aTextColor[3];
} LISTVIEW_ITEM_INFO;
typedef struct {
	WM_HMEM hItemInfo;
	char acText[1];
} LISTVIEW_ITEM;
typedef struct {
	RGB_COLOR                    aBkColor[3];
	RGB_COLOR                    aTextColor[3];
	RGB_COLOR                    GridColor;
	const GUI_FONT GUI_UNI_PTR *pFont;
} LISTVIEW_PROPS;
typedef struct {
	WIDGET          Widget;
	HEADER_Handle   hHeader;
	GUI_ARRAY       RowArray;         /* One entry per line. Every entry is a handle of GUI_ARRAY of strings */
	GUI_ARRAY       AlignArray;       /* One entry per column */
	LISTVIEW_PROPS  Props;
	int             Sel;
	int             ShowGrid;
	unsigned        RowDistY;
	unsigned        LBorder;
	unsigned        RBorder;
	WM_SCROLL_STATE ScrollStateV;
	WM_SCROLL_STATE ScrollStateH;
	WM_HWIN         hOwner;
} LISTVIEW_Obj;
extern LISTVIEW_PROPS LISTVIEW_DefaultProps;
int      LISTVIEW__UpdateScrollParas(LISTVIEW_Obj *pObj);
void     LISTVIEW__InvalidateInsideArea(LISTVIEW_Obj *pObj);
unsigned LISTVIEW__GetRowDistY(const LISTVIEW_Obj *pObj);
void     LISTVIEW__InvalidateRow(LISTVIEW_Obj *pObj, int Sel);
int      LISTVIEW__UpdateScrollPos(LISTVIEW_Obj *pObj);
