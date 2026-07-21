#pragma once

#include "WM.h"
#include "RADIO.h"
#include "WIDGET.h"
#include "GUI_ARRAY.h"

extern CBITMAP RADIO__abmRadio[2];
extern CBITMAP RADIO__bmCheck;

struct RADIO_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC TextColor{ RGB_BLACK };
		RGBC BkColor{ RGB_INVALID_COLOR };
		PCBITMAP apBmRadio[2]{ &RADIO__abmRadio[0], &RADIO__abmRadio[1] };
		PCBITMAP pBmCheck{ &RADIO__bmCheck };
	} static DefaultProps;
	Properties Props;
	GUI_ARRAY TextArray;
	int16_t Sel;                   /* current selection */
	uint16_t Spacing;
	uint16_t Height;
	uint16_t NumItems;
	uint8_t  GroupId;
};

typedef void tRADIO_SetValue(RADIO_Obj *pObj, int v);
extern       tRADIO_SetValue *RADIO__pfHandleSetValue;

void RADIO__SetValue(RADIO_Obj *pObj, int v);
