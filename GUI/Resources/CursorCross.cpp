import TUX.Resources;

#pragma region Large
static BM_BPP2 GUI_Pixels_CrossL[]{
________,________,________,____XXXX,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXoo,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXX__,
XXoooooo,oooooooo,oooooooo,ooooooXX,oooooooo,oooooooo,oooooooo,ooooXX__,
XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXXoo,XXXXXXXX,XXXXXXXX,XXXXXXXX,XXXXXX__,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXoo,XX______,________,________,________,
________,________,________,____XXXX,XX______,________,________,________,
};

static CBITMAP _BitmapCrossL{
	31, 31,              /* XSize, YSize */
	8,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_CrossL,   /* Pointer to picture data (indices) */
	&GUI_CursorPal      /* Pointer to palette */
};
CCURSOR CursorCrossL{ &_BitmapCrossL, 15, 15 };

static CBITMAP _BitmapCrossLI{
	31, 31,              /* XSize, YSize */
	8,                    /* BytesPerLine */
	2,                    /* BitsPerPixel */
	GUI_Pixels_CrossL,    /* Pointer to picture data (indices) */
	&GUI_CursorPalI       /* Pointer to palette */
};
CCURSOR CursorCrossLI{ &_BitmapCrossLI, 15, 15 };
#pragma endregion

#pragma region Median

BM_BPP2 GUI_Pixels_CrossM[]{
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
CCURSOR CursorCrossM{ &_BitmapCrossM, 10, 10 };

static CBITMAP _BitmapCrossMI{
	21, 21,               /* XSize, YSize */
	6,                    /* BytesPerLine */
	2,                    /* BitsPerPixel */
	GUI_Pixels_CrossM,    /* Pointer to picture data (indices) */
	&GUI_CursorPalI       /* Pointer to palette */
};
CCURSOR CursorCrossMI{ &_BitmapCrossMI, 10, 10 };
#pragma endregion

#pragma region Small
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
CCURSOR CursorCrossS{ &_BitmapCrossS, 5, 5 };

static CBITMAP _BitmapCrossSI{
	11, 11,               /* XSize, YSize */
	3,                    /* BytesPerLine */
	2,                    /* BitsPerPixel */
	GUI_Pixels_CrossS,    /* Pointer to picture data (indices) */
	&GUI_CursorPalI       /* Pointer to palette */
};
CCURSOR CursorCrossSI{ &_BitmapCrossSI, 5, 5 };
#pragma endregion
