#pragma once

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

BOOL WM__InitIVRSearch(const GUI_RECT* pMaxRect);
BOOL WM__GetNextIVR(void);

#define WM_ITERATE_START(pRect)   \
	if (WM__InitIVRSearch(pRect)) \
		do {
#define WM_ITERATE_END()          \
	} while (WM__GetNextIVR());

#define WM_ADDORGX(x)    x += GUI_Context.xOff
#define WM_ADDORGY(y)    y += GUI_Context.yOff
#define WM_ADDORG(x0,y0) WM_ADDORGX(x0); WM_ADDORGY(y0)
#define WM_SUBORGX(x)    x -= GUI_Context.xOff
#define WM_SUBORGY(y)    y -= GUI_Context.yOff
#define WM_SUBORG(x0,y0) WM_SUBORGX(x0); WM_SUBORGY(y0)

#if defined(__cplusplus)
}
#endif
