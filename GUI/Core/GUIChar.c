

#include <stddef.h>           /* needed for definition of NULL */
#include <stdio.h>
#include "GUI_Protected.h"
#include "LCD_Private.h"

void GUI_DispNextLine(void) {

	GUI_Context.DispPosY += GUI_GetFontDistY();
	GUI_Context.DispPosX = GUI_Context.LBorder;

}

void GL_DispChar(uint16_t c) {
	/* check for control characters */
	if (c == '\n') {
		GUI_DispNextLine();
	}
	else {
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
	r = GUI_Context.pAFont->YDist;


	return r;
}

int GUI_GetCharDistX(uint16_t c) {
	int r;

	r = GUI_Context.pAFont->pfGetCharDistX(c);

	return r;
}



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
	const uint8_t *pd;
	int x = GUI_Context.DispPosX;
	int y = GUI_Context.DispPosY;
	/* do some checking if drawing is actually necessary ... */
	const GUI_FONT_MONO *pMono = GUI_Context.pAFont->p.pMono;
	unsigned int FirstChar = pMono->FirstChar;
	/* translate character into 2 characters to display : c0,c1 */
	/* Check if regular character first. */
	if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pMono->LastChar)) {
		pd = pMono->pData;
		c0 = ((int)c) - FirstChar;
		c1 = -1;
	}
	else {
		/* Check if character is in translation table */
		GUI_FONT_TRANSINFO const *pti = pMono->pTrans;
		pd = pMono->pTransData;
		if (pti) {
			FirstChar = pti->FirstChar;
			if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pti->LastChar)) {
				GUI_FONT_TRANSLIST const *ptl;
				c -= pti->FirstChar;
				ptl = pti->pList;
				ptl += c;
				c0 = ptl->c0;
				c1 = ptl->c1;
			}
			else {
				c0 = c1 = -1;
			}
		}
		else {
			c0 = c1 = -1;
		}
	}
	/* Draw first character if it is valid */
	if (c0 != -1) {
		int BytesPerChar = GUI_Context.pAFont->YSize * pMono->BytesPerLine;
		GUI_DRAWMODE DrawMode;
		int XSize = pMono->XSize;
		int YSize = GUI_Context.pAFont->YSize;
		/* Select the right drawing mode */
		DrawMode = GUI_Context.TextMode;
		/* call drawing routine */
		{
			uint8_t OldMode = LCD_SetDrawMode(DrawMode);
			LCD_DrawBitmap(x, y,
						   XSize, YSize,
						   1,     /* Bits per Pixel */
						   pMono->BytesPerLine,
						   pd + c0 * BytesPerChar,
						   &LCD_BKCOLORINDEX
			);
			if (c1 != -1) {
				LCD_SetDrawMode(DrawMode | DRAWMODE_TRANS);
				LCD_DrawBitmap(x, y,
							   XSize, YSize,
							   1,     /* Bits per Pixel */
							   pMono->BytesPerLine,
							   pd + c1 * BytesPerChar,
							   &LCD_BKCOLORINDEX
				);
			}
			/* Fill empty pixel lines */
			if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
				if (DrawMode != DRAWMODE_TRANS) {
					LCD_SetDrawMode(DrawMode ^ DRAWMODE_REV);  /* Reverse so we can fill with BkColor */
					LCD_FillRect(x,
								 y + GUI_Context.pAFont->YSize * GUI_Context.pAFont->YDist,
								 x + XSize,
								 y + GUI_Context.pAFont->YDist);
				}
			}
			LCD_SetDrawMode(OldMode);
		}
	}
	//houhh 20061119...
	//  GUI_Context.DispPosX+=pMono->XDist;
	GUI_Context.DispPosX += pMono->XDist;

}

int GUIMONO_GetCharDistX(uint16_t c) {
	const GUI_FONT_MONO *pMono = GUI_Context.pAFont->p.pMono;
	GUI_USE_PARA(c);
	//  return pMono->XDist;
	//houhh 20061119...
	return pMono->XDist;

}

void GUIMONO_GetFontInfo(const GUI_FONT *pFont, GUI_FONTINFO *pfi) {
	GUI_USE_PARA(pFont);
	pfi->Flags = GUI_FONTINFO_FLAG_MONO;
}

char GUIMONO_IsInFont(const GUI_FONT *pFont, uint16_t c) {
	const GUI_FONT_MONO *pMono = pFont->p.pMono;
	unsigned int FirstChar = pMono->FirstChar;
	/* Check if regular character first. */
	if ((c >= (uint16_t)FirstChar) && (c <= (uint16_t)pMono->LastChar)) {
		return 1;  /* Yes, we have it ! */
	}
	else {
		/* Check if character is in translation table */
		GUI_FONT_TRANSINFO const *pti;
		pti = pMono->pTrans;
		if (pti) {
			if ((c >= pti->FirstChar) && (c <= pti->LastChar)) {
				return 1;  /* Yes, we have it ! */
			}
		}
	}
	return 0;  /* No, we can not display this character */
}



#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"


static const GUI_FONT_PROP *GUIPROP_FindChar(const GUI_FONT_PROP *pProp, uint16_t c) {
	for (; pProp; pProp = pProp->pNext) {
		if ((c >= pProp->First) && (c <= pProp->Last))
			break;
	}
	return pProp;
}

/*********************************************************************
*
*       GUIPROP_DispChar
*
* Purpose:
*   This is the routine that displays a character. It is used by all
*   other routines which display characters as a subroutine.
*/
void GUIPROP_DispChar(uint16_t c) {
	int BytesPerLine;

	GUI_DRAWMODE DrawMode = GUI_Context.TextMode;
	const GUI_FONT_PROP *pProp = GUIPROP_FindChar(GUI_Context.pAFont->p.pProp, c);
	if (pProp) {
		GUI_DRAWMODE OldDrawMode;
		const GUI_CHARINFO *pCharInfo = pProp->paCharInfo + (c - pProp->First);
		BytesPerLine = pCharInfo->BytesPerLine;
		OldDrawMode = LCD_SetDrawMode(DrawMode);
		LCD_DrawBitmap(GUI_Context.DispPosX, GUI_Context.DispPosY,
					   pCharInfo->XSize, GUI_Context.pAFont->YSize,
					   1,     /* Bits per Pixel */
					   BytesPerLine,
					   pCharInfo->pData,
					   &LCD_BKCOLORINDEX
		);
		/* Fill empty pixel lines */
		if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
			int YDist = GUI_Context.pAFont->YDist;
			int YSize = GUI_Context.pAFont->YSize;
			if (DrawMode != DRAWMODE_TRANS) {
				RGB_COLOR OldColor = GUI_GetColor();
				GUI_SetColor(GUI_GetBkColor());
				LCD_FillRect(GUI_Context.DispPosX,
							 GUI_Context.DispPosY + YSize,
							 GUI_Context.DispPosX + pCharInfo->XSize,
							 GUI_Context.DispPosY + YDist);
				GUI_SetColor(OldColor);
			}
		}
		LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
		GUI_Context.DispPosX += pCharInfo->XDist;
	}
}

int GUIPROP_GetCharDistX(uint16_t c) {
	const GUI_FONT_PROP *pProp = GUIPROP_FindChar(GUI_Context.pAFont->p.pProp, c);
	return (pProp) ? (pProp->paCharInfo + (c - pProp->First))->XSize : 0;
}

void GUIPROP_GetFontInfo(const GUI_FONT *pFont, GUI_FONTINFO *pfi) {
	GUI_USE_PARA(pFont);
	pfi->Flags = GUI_FONTINFO_FLAG_PROP;
}

char GUIPROP_IsInFont(const GUI_FONT *pFont, uint16_t c) {
	const GUI_FONT_PROP *pProp = GUIPROP_FindChar(pFont->p.pProp, c);
	return (pProp == NULL) ? 0 : 1;
}
