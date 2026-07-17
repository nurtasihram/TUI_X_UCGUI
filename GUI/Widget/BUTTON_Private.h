#pragma once

#include "WM.h"
#include "BUTTON.h"

struct BUTTON_Obj : public WIDGET {
	struct Properties {
		RGB_COLOR aBkColor[3];
		RGB_COLOR aTextColor[3];
		const GUI_FONT  *pFont;
		int16_t Align;
	} Props;
	static Properties DefaultProps;
	char *pText;
	WM_HMEM ahDrawObj[3];
};

void BUTTON__SetDrawObj(BUTTON_Handle hObj, int Index, GUI_DRAW_HANDLE hDrawObj);
