

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"


/*********************************************************************
*
*       GUIMONO_DispChar
*
* Purpose:
*   This is the routine that displays a character. It is used by all
*   other routines which display characters as a subroutine.
*/
void GUIMONO_DispChar(uint16_t c) {
  int c0, c1;
  const uint8_t GUI_UNI_PTR * pd;
  int x = GUI_Context.DispPosX;
  int y = GUI_Context.DispPosY;
  /* do some checking if drawing is actually necessary ... */
  const GUI_FONT_MONO GUI_UNI_PTR * pMono = GUI_Context.pAFont->p.pMono;
  unsigned int FirstChar = pMono->FirstChar;
  /* translate character into 2 characters to display : c0,c1 */
  /* Check if regular character first. */
  if ((c >= (uint16_t)FirstChar) &&(c <= (uint16_t)pMono->LastChar)) {
    pd = pMono->pData;
    c0 = ((int)c) - FirstChar;
    c1 = -1;
  } else {
    /* Check if character is in translation table */
    GUI_FONT_TRANSINFO const GUI_UNI_PTR * pti = pMono->pTrans;
    pd = pMono->pTransData;
    if (pti) {
      FirstChar = pti->FirstChar;
      if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pti->LastChar)) {
        GUI_FONT_TRANSLIST const GUI_UNI_PTR * ptl;
        c -= pti->FirstChar;
        ptl = pti->pList;
        ptl += c;
        c0  = ptl->c0;
        c1  = ptl->c1;
      } else {
        c0 = c1 = -1;
      }
    } else {
      c0 = c1 = -1;
    }
  }
  /* Draw first character if it is valid */
  if (c0!=-1) {
    int BytesPerChar = GUI_Context.pAFont->YSize*pMono->BytesPerLine;
    GUI_DRAWMODE DrawMode;
    int XSize = pMono->XSize;
    int YSize = GUI_Context.pAFont->YSize;
    /* Select the right drawing mode */
    DrawMode = GUI_Context.TextMode;
    /* call drawing routine */
    {
      uint8_t OldMode = LCD_SetDrawMode(DrawMode);
      LCD_DrawBitmap( x, y,
                         XSize, YSize,
                         GUI_Context.pAFont->XMag,  GUI_Context.pAFont->YMag,
                         1,     /* Bits per Pixel */
                         pMono->BytesPerLine,
                         pd + c0* BytesPerChar,
                         &LCD_BKCOLORINDEX
                         );
      if (c1 != -1) {
        LCD_SetDrawMode(DrawMode | LCD_DRAWMODE_TRANS);
        LCD_DrawBitmap( x, y,
                           XSize, YSize,
                           GUI_Context.pAFont->XMag,  GUI_Context.pAFont->YMag,
                           1,     /* Bits per Pixel */
                           pMono->BytesPerLine,
                           pd + c1* BytesPerChar,
                           &LCD_BKCOLORINDEX
                           );
      }
      /* Fill empty pixel lines */
      if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
        if (DrawMode != LCD_DRAWMODE_TRANS) {
          LCD_SetDrawMode(DrawMode ^ LCD_DRAWMODE_REV);  /* Reverse so we can fill with BkColor */
          LCD_FillRect(x,
                       y + GUI_Context.pAFont->YSize * GUI_Context.pAFont->YDist,
                       x + XSize * GUI_Context.pAFont->XMag,
                       y + GUI_Context.pAFont->YDist);
        }
      }
      LCD_SetDrawMode(OldMode);
    }
  }
//houhh 20061119...
//  GUI_Context.DispPosX+=pMono->XDist;
  GUI_Context.DispPosX+=pMono->XDist * GUI_Context.pAFont->XMag;

}

int GUIMONO_GetCharDistX(uint16_t c) {
  const GUI_FONT_MONO GUI_UNI_PTR * pMono = GUI_Context.pAFont->p.pMono;
  GUI_USE_PARA(c);
//  return pMono->XDist;
//houhh 20061119...
  return pMono->XDist*GUI_Context.pAFont->XMag;

}

void GUIMONO_GetFontInfo(const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO* pfi) {
  GUI_USE_PARA(pFont);
  pfi->Flags = GUI_FONTINFO_FLAG_MONO;
}

char GUIMONO_IsInFont(const GUI_FONT GUI_UNI_PTR * pFont, uint16_t c) {
  const GUI_FONT_MONO GUI_UNI_PTR * pMono = pFont->p.pMono;
  unsigned int FirstChar = pMono->FirstChar;
  /* Check if regular character first. */
  if ((c >= (uint16_t)FirstChar) &&(c <= (uint16_t)pMono->LastChar)) {
    return 1;  /* Yes, we have it ! */
  } else {
   /* Check if character is in translation table */
    GUI_FONT_TRANSINFO const GUI_UNI_PTR * pti;
    pti = pMono->pTrans;
    if (pti) {
      if ((c >= pti->FirstChar) && (c <= pti->LastChar)) {
        return 1;  /* Yes, we have it ! */
      }
    }
  }
  return 0;  /* No, we can not display this character */
}

/*************************** End of file ****************************/
