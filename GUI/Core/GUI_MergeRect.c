

#include "GUI.h"
#include "GUIDebug.h"

#define Min(v0,v1) ((v0>v1) ? v1 : v0)
#define Max(v0,v1) ((v0>v1) ? v0 : v1)

/*********************************************************************
*
*       GUI_MergeRect
*
* Purpose:
*   Calc smalles rectangles containing both rects.
*/
void GUI_MergeRect(GUI_RECT* pDest, const GUI_RECT* pr0, const GUI_RECT* pr1) {
  if (pDest) {
    if (pr0 && pr1) {
      pDest->x0 = Min(pr0->x0, pr1->x0);
      pDest->y0 = Min(pr0->y0, pr1->y0);
      pDest->x1 = Max(pr0->x1, pr1->x1);
      pDest->y1 = Max(pr0->y1, pr1->y1);
      return;
    }
    *pDest = *(pr0 ? pr0 : pr1);
  }
}

/*************************** End of file ****************************/
