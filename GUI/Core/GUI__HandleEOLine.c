

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Protected.h"
#include "GUIDebug.h"

/*********************************************************************
*
*       GUI__HandleEOLine
*
* Is called when processing strings which may consist of
* multiple lines after a line has been processed. It will
* a) Swall the line feed character (if it is there)
* b) Return 1 if end of string, otherwise 0
*/
int GUI__HandleEOLine(const char GUI_UNI_PTR **ps) {
  const char GUI_UNI_PTR *s = *ps;
  char c = *s++;
  if (c == 0) {
    return 1;
  }
  if (c == '\n') {
    *ps = s;
  }
  return 0;
}


