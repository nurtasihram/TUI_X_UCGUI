#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"

/*********************************************************************
*
*       GUI__strcmp
*
* Purpose:
*   Replacement for the strcmp function. The advantage is that it can
*   be called with NULL pointer(s).
*   A NULL string is treated as a an empty string ("").
* Return value:
*   0 if identical
*   1 else
*/
int GUI__strcmp(const char  * s0, const char  * s1) {
  if (s0 == NULL) {
    s0 = "";
  }
  if (s1 == NULL) {
    s1 = "";
  }
  do {
    if (*s0 != *s1) {
      return 1;
    }
    s1++;
  } while (*++s0);
  if (*s1) {
    return 1;    /* Not equal, since s1 is longer than s0 */
  }
  return 0;      /* Equal ! */
}
