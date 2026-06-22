

#include "GUI_Protected.h"

/*********************************************************************
*
*       GUI__IntersectRects
*
* Purpose:
*   Calc intersection of rectangles
*
* Add. info:
*   Rectangles are passed as pointers. These pointers need to be valid;
*   a NULL pointer may not be passed. There is no check for NULL pointers
*   implemented in order to avoid avoid performance penalty.
*   There is a similar function available, GUI__IntersectRects(),
*   which takes 3 parameters and
*   has a return value. Note that this one should be preferred because
*   it is considerably faster and the call requires one parameter less.
*/
void GUI__IntersectRect(GUI_RECT* pDest, const GUI_RECT* pr0) {
  if (pDest->x0 < pr0->x0) {
    pDest->x0 = pr0->x0;
  }
  if (pDest->y0 < pr0->y0) {
    pDest->y0 = pr0->y0;
  }
  if (pDest->x1 > pr0->x1) {
    pDest->x1 = pr0->x1;
  }
  if (pDest->y1 > pr0->y1) {
    pDest->y1 = pr0->y1;
  }
}


