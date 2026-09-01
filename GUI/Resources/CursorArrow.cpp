import TUX.Resources;

#pragma region Large
static BM_BPP2 _pxArrowL[]{
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
________,________,______XX,XXXX____,________};
static CBITMAP _bmArrowL{ { 18, 30 }, 5, 2, _pxArrowL, GUI_CursorPal };
CCURSOR CursorArrowL{ &_bmArrowL };
static CBITMAP _bmArrowLI{ { 18, 30 }, 5, 2, _pxArrowL, GUI_CursorPalI };
CCURSOR CursorArrowLI{ &_bmArrowLI };
#pragma endregion

#pragma region Median
static BM_BPP2 _pxArrowM[]{
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
________,______XX,XXXX____};
static CBITMAP _bmArrowM{ { 12, 20 }, 3, 2, _pxArrowM, GUI_CursorPal };
CCURSOR CursorArrowM{ &_bmArrowM };
static CBITMAP _bmArrowMI{ { 12, 20 }, 3, 2, _pxArrowM, GUI_CursorPalI };
CCURSOR CursorArrowMI{ &_bmArrowMI };
#pragma endregion

#pragma region Small
static BM_BPP2 _pxArrowS[]{
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
________,__XXXX__,________};
static CBITMAP _bmArrowS{ { 9, 15 }, 3, 2, _pxArrowS, GUI_CursorPal };
CCURSOR CursorArrowS{ &_bmArrowS };
static CBITMAP _bmArrowSI{ { 9, 15 }, 3, 2, _pxArrowS, GUI_CursorPalI };
CCURSOR CursorArrowSI{ &_bmArrowSI };
#pragma endregion
