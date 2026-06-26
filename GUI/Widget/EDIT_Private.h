#pragma once
#include "WIDGET.h"
#define EDIT_REALLOC_SIZE  16
typedef struct EDIT_Obj_struct EDIT_Obj;
typedef struct {
	int                          Align;
	int                          Border;
	const GUI_FONT  *pFont;
	RGB_COLOR                    aTextColor[2];
	RGB_COLOR                    aBkColor[2];
} EDIT_PROPS;
struct EDIT_Obj_struct {
	WIDGET Widget;
	WM_HMEM hpText;
	int16_t MaxLen;
	uint16_t BufferSize;
	int32_t Min, Max;            /* Min max values as normalized floats (integers) */
	uint8_t NumDecs;              /* Number of decimals */
	uint32_t CurrentValue;        /* Current value */
	int CursorPos;           /* Cursor position. 0 means left most */
	unsigned SelSize;        /* Number of selected characters */
	uint8_t EditMode;             /* Insert or overwrite mode */
	uint8_t XSizeCursor;          /* Size of cursor when working in insert mode */
	uint8_t Flags;
	tEDIT_AddKeyEx *pfAddKeyEx;     /* Handle key input */
	tEDIT_UpdateBuffer *pfUpdateBuffer;  /* Update textbuffer */
	EDIT_PROPS Props;
	int CurrsorShow;	//houhh 20061022...
};

extern EDIT_PROPS EDIT__DefaultProps;
void EDIT__SetCursorPos(EDIT_Obj *pObj, int CursorPos);
uint16_t  EDIT__GetCurrentChar(EDIT_Obj *pObj);
