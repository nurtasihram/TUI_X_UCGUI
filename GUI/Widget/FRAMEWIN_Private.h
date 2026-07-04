#pragma once

#include "WM.h"
#include "FRAMEWIN.h"
#include "WIDGET.h"

typedef struct {
	const GUI_FONT *pFont;
	RGB_COLOR  aBarColor[2];
	RGB_COLOR  aTextColor[2];
	RGB_COLOR  ClientColor;
	int16_t    TitleHeight;
	int16_t    BorderSize;
	int16_t    IBorderSize;
} FRAMEWIN_PROPS;

typedef struct {
	WIDGET Widget;
	FRAMEWIN_PROPS Props;
	WM_CALLBACK *cb;
	WM_HWIN hClient;
	WM_HWIN hMenu;
	WM_HWIN hText;
	GUI_RECT rRestore;
	int16_t TextAlign;
	uint16_t Flags;
	WM_HWIN hFocussedChild;          /* Handle to focussed child .. default none (0) */
	WM_DIALOG_STATUS *pDialogStatus;
} FRAMEWIN_Obj;

typedef struct {
	int16_t TitleHeight;
	int16_t MenuHeight;
	GUI_RECT rClient;
	GUI_RECT rTitleText;
} POSITIONS;

extern FRAMEWIN_PROPS FRAMEWIN__DefaultProps;

void            FRAMEWIN__CalcPositions(FRAMEWIN_Obj *pObj, POSITIONS *pPos);
int             FRAMEWIN__CalcTitleHeight(FRAMEWIN_Obj *pObj);
void            FRAMEWIN__UpdatePositions(FRAMEWIN_Obj *pObj);
void            FRAMEWIN__UpdateButtons(FRAMEWIN_Obj *pObj, int OldHeight);
const GUI_FONT *FRAMEWIN_GetFont(FRAMEWIN_Handle hObj);
int             FRAMEWIN_GetTitleHeight(FRAMEWIN_Handle hObj);
void            FRAMEWIN_MinButtonSetState(WM_HWIN hButton, int State);
void            FRAMEWIN_MaxButtonSetState(WM_HWIN hButton, int State);
