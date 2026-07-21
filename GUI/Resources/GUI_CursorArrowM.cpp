#include "GUI_Protected.h"

const uint8_t GUI_Pixels_ArrowM[]{
XX______,________,________,
XXXX____,________,________,
XXooXX__,________,________,
XXooooXX,________,________,
XXoooooo,XX______,________,
XXoooooo,ooXX____,________,
XXoooooo,ooooXX__,________,
XXoooooo,ooooooXX,________,
XXoooooo,oooooooo,XX______,
XXoooooo,oooooooo,ooXX____,
XXoooooo,oooooooo,ooooXX__,
XXoooooo,ooooooXX,XXXXXXXX,
XXooooXX,ooooooXX,________,
XXooXX__,XXoooooo,XX______,
XXXX____,XXoooooo,XX______,
XX______,__XXoooo,ooXX____,
________,__XXoooo,ooXX____,
________,____XXoo,ooooXX__,
________,____XXoo,ooooXX__,
________,______XX,XXXX____,
};

static CBITMAP _BitmapArrowM{
	12, 29,              /* XSize, YSize */
	3,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowM,   /* Pointer to picture data (indices) */
	&GUI_CursorPal       /* Pointer to palette */
};
extern CCURSOR GUI_CursorArrowM{ &_BitmapArrowM, 0, 0 };

static CBITMAP _BitmapArrowMI{
	12, 29,              /* XSize, YSize */
	3,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowM,   /* Pointer to picture data (indices) */
	&GUI_CursorPalI      /* Pointer to palette */
};
extern CCURSOR GUI_CursorArrowMI{ &_BitmapArrowMI, 0, 0 };
