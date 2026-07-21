#pragma once

#include "TEXT.h"
#include "WIDGET.h"

struct TEXT_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		TEXTALIGN Align{ 0 };
		RGBC TextColor{ RGB_BLACK };
		RGBC BkColor{ RGB_INVALID_COLOR };
	} static DefaultProps;
	Properties Props;
	char *pText;
};
