#pragma once

#include "WIDGET.h"
#include "HEADER.h"
#include "WM.h"
#include "GUI_ARRAY.h"

struct HEADER_COLUMN {
	int16_t Width;
	TEXTALIGN Align;
	GUI_DRAW *pDrawObj;
	char acText[1];
};

struct HEADER_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC BkColor{ RGB_GRAYL(0xAA) };
		RGBC TextColor{ RGB_BLACK };
	} static DefaultProps;
	Properties Props;
	GUI_ARRAY Columns;
	int16_t CapturePosX = -1;
	int16_t CaptureItem = -1;
	uint16_t ScrollPos = 0;
};

void HEADER__SetDrawObj(HEADER_Handle hObj, unsigned Index, GUI_DRAW *pDrawObj);
