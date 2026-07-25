#pragma once

#include "DIALOG_Intern.h"

#include "WM_Intern.h"

import TUX.Widget;
import TUX.Widget.Slider;

struct SLIDER_Obj : public WIDGET {
	struct Properties {
		RGBC BkColor{ RGB_GRAYL(0xC0) };
		RGBC Color{ RGB_GRAYL(0xC0) };
	} static DefaultProps;
	Properties Props;
	int16_t Min, Max, v;
	int16_t NumTicks;
	int16_t Width;
	uint8_t Flags;
};
