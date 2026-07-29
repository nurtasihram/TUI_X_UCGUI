import TUX.Resources;

#pragma region Large
static BM_BPP2 GUI_Pixels_ArrowL[]{
XX______,________,________,________,________,
XXXX____,________,________,________,________,
XXooXX__,________,________,________,________,
XXooooXX,________,________,________,________,
XXoooooo,XX______,________,________,________,
XXoooooo,ooXX____,________,________,________,
XXoooooo,ooooXX__,________,________,________,
XXoooooo,ooooooXX,________,________,________,
XXoooooo,oooooooo,XX______,________,________,
XXoooooo,oooooooo,ooXX____,________,________,
XXoooooo,oooooooo,ooooXX__,________,________,
XXoooooo,oooooooo,ooooooXX,________,________,
XXoooooo,oooooooo,oooooooo,XX______,________,
XXoooooo,oooooooo,oooooooo,ooXX____,________,
XXoooooo,oooooooo,oooooooo,ooooXX__,________,
XXoooooo,oooooooo,oooooooo,ooooooXX,________,
XXoooooo,oooooooo,oooooooo,oooooooo,XX______,
XXoooooo,oooooooo,ooXXXXXX,XXXXXXXX,XXXX____,
XXoooooo,ooXXoooo,ooXX____,________,________,
XXoooooo,XX__XXoo,ooooXX__,________,________,
XXooooXX,____XXoo,ooooXX__,________,________,
XXooXX__,______XX,ooooooXX,________,________,
XXXX____,______XX,ooooooXX,________,________,
XX______,________,XXoooooo,XX______,________,
________,________,XXoooooo,XX______,________,
________,________,__XXoooo,ooXX____,________,
________,________,__XXoooo,ooXX____,________,
________,________,____XXoo,ooooXX__,________,
________,________,____XXoo,ooooXX__,________,
________,________,______XX,XXXX____,________,
};

static CBITMAP _BitmapArrowL{
	18, 30,              /* XSize, YSize */
	5,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowL,   /* Pointer to picture data (indices) */
	&GUI_CursorPal       /* Pointer to palette */
};
CCURSOR CursorArrowL{ &_BitmapArrowL, 0, 0 };

static CBITMAP _BitmapArrowLI{
	18, 30,              /* XSize, YSize */
	5,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowL,   /* Pointer to picture data (indices) */
	&GUI_CursorPalI      /* Pointer to palette */
};
CCURSOR CursorArrowLI{ &_BitmapArrowLI, 0, 0 };
#pragma endregion

#pragma region Median
static BM_BPP2 GUI_Pixels_ArrowM[]{
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
	12, 20,              /* XSize, YSize */
	3,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowM,   /* Pointer to picture data (indices) */
	&GUI_CursorPal       /* Pointer to palette */
};
CCURSOR CursorArrowM{ &_BitmapArrowM, 0, 0 };

static CBITMAP _BitmapArrowMI{
	12, 20,              /* XSize, YSize */
	3,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowM,   /* Pointer to picture data (indices) */
	&GUI_CursorPalI      /* Pointer to palette */
};
CCURSOR CursorArrowMI{ &_BitmapArrowMI, 0, 0 };
#pragma endregion

#pragma region Small
static BM_BPP2 GUI_Pixels_ArrowS[]{
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
CCURSOR CursorArrowS{ &_BitmapArrowS, 0, 0 };

static CBITMAP _BitmapArrowSI{
	9, 15,               /* XSize, YSize */
	3,                   /* BytesPerLine */
	2,                   /* BitsPerPixel */
	GUI_Pixels_ArrowS,   /* Pointer to picture data (indices) */
	&GUI_CursorPalI      /* Pointer to palette */
};
CCURSOR CursorArrowSI{ &_BitmapArrowSI, 0, 0 };
#pragma endregion
