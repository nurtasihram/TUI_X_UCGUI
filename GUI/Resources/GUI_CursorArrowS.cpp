#include "GUI_Protected.h"

const uint8_t GUI_Pixels_ArrowS[]{
XX______,________,________,
XXXX____,________,________,
XXooXX__,________,________,
XXooooXX,________,________,
XXoooooo,XX______,________,
XXoooooo,ooXX____,________,
XXoooooo,ooooXX__,________,
XXoooooo,ooooooXX,________,
XXoooooo,ooXXXXXX,XX______,
XXooXXoo,ooXX____,________,
XXXX__XX,ooooXX__,________,
XX____XX,ooooXX__,________,
________,XXooooXX,________,
________,XXooooXX,________,
________,__XXXX__,________,
};

static CBITMAP _BitmapArrowS{
	9, 15,               /* XSize, YSize */
	3,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowS,   /* Pointer to picture data (indices) */
	&GUI_CursorPal       /* Pointer to palette */
};
extern CCURSOR GUI_CursorArrowS{ &_BitmapArrowS, 0, 0 };

static CBITMAP _BitmapArrowSI{
	9, 15,               /* XSize, YSize */
	3,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowS,   /* Pointer to picture data (indices) */
	&GUI_CursorPalI      /* Pointer to palette */
};
extern CCURSOR GUI_CursorArrowSI{ &_BitmapArrowSI, 0, 0 };
