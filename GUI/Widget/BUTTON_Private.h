#pragma once

#include "WM.h"
#include "BUTTON.h"

struct BUTTON_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC aTextColor[3]{
			/* Unpressed */	RGB_BLACK,
			/* Pressed */	RGB_BLACK,
			/* Disabled */	RGB_DARKGRAY
		};
		RGBC aBkColor[3]{
			/* Unpressed */	RGB_GRAYL(0xD0),
			/* Pressed */	RGB_WHITE,
			/* Disabled */	RGB_LIGHTGRAY
		};
		TEXTALIGN Align{ TEXTALIGN_CENTER };
	} static DefaultProps;
	Properties Props;
	char *pText;
	GUI_DRAW *aDrawObj[3];
};

void BUTTON__SetDrawObj(BUTTON_Handle hObj, int Index, GUI_DRAW *pDrawObj);
