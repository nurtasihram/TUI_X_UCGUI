#pragma once

#include "WM.h"
#include "BUTTON.h"

struct BUTTON_Obj : public WIDGET {
	struct Properties {
		const GUI_FONT *pFont{ &GUI_Font13_1 };
		RGB_COLOR aTextColor[3]{
			/* Unpressed */	RGB_BLACK,
			/* Pressed */	RGB_BLACK,
			/* Disabled */	RGB_DARKGRAY
		};
		RGB_COLOR aBkColor[3]{
			/* Unpressed */	RGB_GRAYL(0xD0),
			/* Pressed */	RGB_WHITE,
			/* Disabled */	RGB_LIGHTGRAY
		};
		TEXTALIGN Align{ TEXTALIGN_CENTER };
	} static DefaultProps;
	Properties Props;
	char *pText;
	WM_HMEM ahDrawObj[3];
};

void BUTTON__SetDrawObj(BUTTON_Handle hObj, int Index, GUI_DRAW_HANDLE hDrawObj);
