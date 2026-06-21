
#include "FRAMEWIN.h"

FRAMEWIN_Handle FRAMEWIN_Create(const char* pText, WM_CALLBACK* cb, int Flags,
                                int x0, int y0, int xsize, int ysize) {
  return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, WM_HWIN_NULL, Flags, 0, 0, pText, cb);
}
FRAMEWIN_Handle FRAMEWIN_CreateAsChild(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                       const char* pText, WM_CALLBACK* cb, int Flags) {
  return FRAMEWIN_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, 0, pText, cb);
}

