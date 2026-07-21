#pragma once

#include "MULTIEDIT.h"
#include "WIDGET.h"

#define NUM_DISP_MODES 2

struct MULTIEDIT_Obj : public WIDGET {
	struct Properties {
		PCFONT pFont{ &GUI_Font13_1 };
		RGBC aBkColor[NUM_DISP_MODES]{
			/* Edit mode */		RGB_WHITE,
			/* Read-only */		RGB_GRAYL(0xC0)
		};
		RGBC aColor[NUM_DISP_MODES]{
			/* Edit mode */		RGB_BLACK,
			/* Read-only */		RGB_BLACK
		};
		uint8_t HBorder{ 1 };
	} static DefaultProps;
	Properties Props;
	WM_HMEM hText;
	uint16_t MaxNumChars;         /* Maximum number of characters including the prompt */
	uint16_t NumChars;            /* Number of characters (text and prompt) in object */
	uint16_t NumCharsPrompt;      /* Number of prompt characters */
	uint16_t NumLines;            /* Number of text lines needed to show all data */
	uint16_t TextSizeX;           /* Size in X of text depending of wrapping mode */
	uint16_t BufferSize;
	uint16_t CursorLine;          /* Number of current cursor line */
	uint16_t CursorPosChar;       /* Character offset number of cursor */
	uint16_t CursorPosByte;       /* Byte offset number of cursor */
	uint16_t CursorPosX;          /* Cursor position in X */
	uint16_t CursorPosY;          /* Cursor position in Y */
	uint16_t CacheLinePosByte;    /*  */
	uint16_t CacheLineNumber;     /*  */
	uint16_t CacheFirstVisibleLine;
	uint16_t CacheFirstVisibleByte;
	WM_SCROLL_STATE ScrollStateV;
	WM_SCROLL_STATE ScrollStateH;
	uint8_t Flags;
	uint8_t InvalidFlags;         /* Flags to save validation status */
	uint8_t EditMode;
	GUI_WRAPMODE WrapMode;
};
