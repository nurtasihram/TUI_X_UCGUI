#pragma once

#include "WM.h"
#include "WIDGET.h"
#include "CHECKBOX.h"

struct CHECKBOX_Obj : public WIDGET {
	static CBITMAP abmCheck[2];
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		PCBITMAP apBm[4]{
			/* Inactive */	&abmCheck[0],
			/* Active */	&abmCheck[1],
			/* Inactive 3-State */	&abmCheck[2],
			/* Active 3-State */	&abmCheck[1]
		};
		RGBC aBkColorBox[2]{
			/* Inactive */	RGB_GRAYL(0x80),
			/* Active */	RGB_WHITE
		};
		RGBC BkColor{ RGB_INVALID_COLOR };
		RGBC TextColor{ RGB_BLACK };
		TEXTALIGN Align{ TEXTALIGN_LEFT | TEXTALIGN_VCENTER };
		uint8_t Spacing{ 4 };
		uint8_t NumStates = 2;
	} static DefaultProps;
	Properties Props;
	uint8_t NumStates;
	uint8_t CurrentState;
	char *pText;
};
