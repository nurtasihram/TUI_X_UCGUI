#pragma once

#include "WM.h"
#include "BUTTON.h"

typedef struct {
	RGB_COLOR aBkColor[3];
	RGB_COLOR aTextColor[3];
	const GUI_FONT  *pFont;
	int16_t Align;
} BUTTON_PROPS;

struct BUTTON_Obj : public WIDGET {
	BUTTON_PROPS Props;
	char *pText;
	WM_HMEM ahDrawObj[3];
};

extern BUTTON_PROPS BUTTON__DefaultProps;

void BUTTON__SetDrawObj(BUTTON_Handle hObj, int Index, GUI_DRAW_HANDLE hDrawObj);
