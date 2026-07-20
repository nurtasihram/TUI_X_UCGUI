#pragma once

#include "WM.h"
#include "FRAMEWIN.h"
#include "WIDGET.h"

struct FRAMEWIN_Obj : public WIDGET {
	struct Properties {
		const GUI_FONT *pFont{ &GUI_Font13_1 };
		RGB_COLOR aTextColor[2]{
			/* Lose focused */	RGB_BLACK,
			/* Focused */		RGB_WHITE
		};
		RGB_COLOR aBarColor[2]{
			/* Lose focused */	RGB_GRAYL(0x80),
			/* Focused */		RGB_BLUEL(0x80)
		};
		RGB_COLOR ClientColor{ RGB_GRAYL(0xE4) };
		RGB_COLOR FrameColor{ RGB_GRAYL(0xAA) };
		uint16_t TitleHeight{ 20 };
		uint16_t BorderSize{ 2 };
		uint16_t IBorderSize{ 1 };
		TEXTALIGN Align{ TEXTALIGN_VCENTER };
		uint8_t Border{ 0 };
	} static DefaultProps;
	Properties Props;
	WM_CALLBACK *cb;
	WM_HWIN hClient;
	WM_HWIN hMenu;
	char *pText;
	GUI_RECT rRestore;
	uint16_t Flags;
	WM_HWIN hFocussedChild; /* Handle to focussed child .. default none (0) */
	WM_DIALOG_STATUS *pDialogStatus;
};

struct POSITIONS {
	int16_t TitleHeight;
	int16_t MenuHeight;
	GUI_RECT rClient;
	GUI_RECT rTitleText;
} ;

void            FRAMEWIN__CalcPositions(FRAMEWIN_Obj *pObj, POSITIONS *pPos);
int             FRAMEWIN__CalcTitleHeight(FRAMEWIN_Obj *pObj);
void            FRAMEWIN__UpdatePositions(FRAMEWIN_Obj *pObj);
void            FRAMEWIN__UpdateButtons(FRAMEWIN_Obj *pObj, int OldHeight);
const GUI_FONT *FRAMEWIN_GetFont(FRAMEWIN_Handle hObj);
int             FRAMEWIN_GetTitleHeight(FRAMEWIN_Handle hObj);
void            FRAMEWIN_MinButtonSetState(WM_HWIN hButton, int State);
void            FRAMEWIN_MaxButtonSetState(WM_HWIN hButton, int State);
