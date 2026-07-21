#include "GUI_Protected.h"

const uint8_t GUI_Pixels_CrossM[]{
________,________,__XXXXXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
XXXXXXXX,XXXXXXXX,XXXXooXX,XXXXXXXX,XXXXXXXX,XX______,
XXoooooo,oooooooo,ooooXXoo,oooooooo,oooooooo,XX______,
XXXXXXXX,XXXXXXXX,XXXXooXX,XXXXXXXX,XXXXXXXX,XX______,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXooXX,________,________,________,
________,________,__XXXXXX,________,________,________,
};

static CBITMAP _BitmapCrossM{
	21, 21,               /* XSize, YSize */
	6,                    /* BytesPerLine */
	2,                    /* BitsPerPixel */
	GUI_Pixels_CrossM,    /* Pointer to picture data (indices) */
	&GUI_CursorPal        /* Pointer to palette */
};
extern CCURSOR GUI_CursorCrossM{ &_BitmapCrossM, 10, 10 };

static CBITMAP _BitmapCrossMI{
	21, 21,               /* XSize, YSize */
	6,                    /* BytesPerLine */
	2,                    /* BitsPerPixel */
	GUI_Pixels_CrossM,    /* Pointer to picture data (indices) */
	&GUI_CursorPalI       /* Pointer to palette */
};
extern CCURSOR GUI_CursorCrossMI{ &_BitmapCrossMI, 10, 10 };
