#pragma once

#include "WM.h"
#include "RADIO.h"
#include "WIDGET.h"
#include "GUI_ARRAY.h"


extern CBITMAP RADIO__abmRadio[2];
extern CBITMAP RADIO__bmCheck;
extern PCBITMAP RADIO__apDefaultImage[2];
extern PCBITMAP RADIO__pDefaultImageCheck;
extern PCFONT RADIO__pDefaultFont;
extern       RGBC RADIO__DefaultTextColor;

struct RADIO_Obj : public WIDGET {
	PCBITMAP apBmRadio[2];
	PCBITMAP pBmCheck;
	GUI_ARRAY TextArray;
	int16_t Sel;                   /* current selection */
	uint16_t Spacing;
	uint16_t Height;
	uint16_t NumItems;
	uint8_t  GroupId;
	RGBC BkColor;
	RGBC TextColor;
	PCFONT pFont;
};

typedef void tRADIO_SetValue(RADIO_Obj *pObj, int v);
extern       tRADIO_SetValue *RADIO__pfHandleSetValue;

void RADIO__SetValue(RADIO_Obj *pObj, int v);
