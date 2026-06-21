

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include "GUI_Protected.h"


void GUI_DispNextLine(void) {

  GUI_Context.DispPosY += GUI_GetFontDistY();
  GUI_Context.DispPosX  = GUI_Context.LBorder;

}

void GL_DispChar(uint16_t c) {
  /* check for control characters */
  if (c == '\n') {
    GUI_DispNextLine();
  } else {
    if (c != '\r') {

      GUI_Context.pAFont->pfDispChar(c);
      if (GUI_pfDispCharStyle) {
        GUI_pfDispCharStyle(c);
      }

    }
  }
}

/*********************************************************************
*
*       GUI_GetYAdjust
*
* Returns adjustment in vertical (Y) direction
*
* Note: The return value needs to be subtracted from
*       the y-position of the character.
*/
int GUI_GetYAdjust(void) {
  int r = 0;

  switch (GUI_Context.TextAlign & GUI_TA_VERTICAL) {
	case GUI_TA_BOTTOM:
		r = GUI_Context.pAFont->YSize - 1;
    break;
	case GUI_TA_VCENTER:
		r = GUI_Context.pAFont->YSize / 2;
    break;
	case GUI_TA_BASELINE:
		r = GUI_Context.pAFont->YSize / 2;
	}

  return r;
}

int GUI_GetFontDistY(void) {
  int r;

//  r = GUI_Context.pAFont->YDist;
  r = GUI_Context.pAFont->YDist * GUI_Context.pAFont->YMag;


  return r;
}

int GUI_GetCharDistX(uint16_t c) {
  int r;

  r = GUI_Context.pAFont->pfGetCharDistX(c);

  return r;
}

/*************************** End of file ****************************/
