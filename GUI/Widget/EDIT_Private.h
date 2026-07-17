#pragma once

#include "WIDGET.h"

#define EDIT_REALLOC_SIZE  16

struct EDIT_Obj : public WIDGET {
	struct Properties {
		int16_t Align;
		int16_t Border;
		const GUI_FONT *pFont;
		RGB_COLOR aTextColor[2];
		RGB_COLOR aBkColor[2];
	} Props;
	static Properties DefaultProps;
	char *pText;
	int16_t MaxLen;
	uint16_t BufferSize;
	int32_t Min, Max;            /* Min max values as normalized floats (integers) */
	uint8_t NumDecs;              /* Number of decimals */
	uint32_t CurrentValue;        /* Current value */
	int16_t CursorPos;           /* Cursor position. 0 means left most */
	uint16_t SelSize;        /* Number of selected characters */
	uint8_t EditMode;             /* Insert or overwrite mode */
	uint8_t XSizeCursor;          /* Size of cursor when working in insert mode */
	uint8_t Flags;
	tEDIT_AddKeyEx *pfAddKeyEx;     /* Handle key input */
	tEDIT_UpdateBuffer *pfUpdateBuffer;  /* Update textbuffer */
	int CurrsorShow;	//houhh 20061022...
};


void EDIT__SetCursorPos(EDIT_Obj *pObj, int CursorPos);
uint16_t  EDIT__GetCurrentChar(EDIT_Obj *pObj);
