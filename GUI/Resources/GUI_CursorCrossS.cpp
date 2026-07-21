#include "GUI_Protected.h"

BM_BPP2 GUI_Pixels_CrossS[]{
________,XXXXXX__,________,
________,XXooXX__,________,
________,XXooXX__,________,
________,XXooXX__,________,
XXXXXXXX,XXooXXXX,XXXXXX__,
XXoooooo,ooXXoooo,ooooXX__,
XXXXXXXX,XXooXXXX,XXXXXX__,
________,XXooXX__,________,
________,XXooXX__,________,
________,XXooXX__,________,
________,XXXXXX__,________,
};

static CBITMAP _BitmapCrossS{
	11, 11,               /* XSize, YSize */
	3,                    /* BytesPerLine */
	2,                    /* BitsPerPixel */
	GUI_Pixels_CrossS,    /* Pointer to picture data (indices) */
	&GUI_CursorPal        /* Pointer to palette */
};
extern CCURSOR GUI_CursorCrossS{ &_BitmapCrossS, 5, 5 };

static CBITMAP _BitmapCrossSI{
	11, 11,               /* XSize, YSize */
	3,                    /* BytesPerLine */
	2,                    /* BitsPerPixel */
	GUI_Pixels_CrossS,    /* Pointer to picture data (indices) */
	&GUI_CursorPalI       /* Pointer to palette */
};
extern CCURSOR GUI_CursorCrossSI{ &_BitmapCrossSI, 5, 5 };
