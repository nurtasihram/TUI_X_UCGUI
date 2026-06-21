#include <stdlib.h>
#include <string.h>

#include "GUI.h"
#include "GUI_Protected.h"

#include "RADIO.h"
#include "RADIO_Private.h"

/* Define default image inactiv */
#ifndef RADIO_IMAGE0_DEFAULT
#define RADIO_IMAGE0_DEFAULT        &RADIO__abmRadio[0]
#endif
/* Define default image activ */
#ifndef RADIO_IMAGE1_DEFAULT
#define RADIO_IMAGE1_DEFAULT        &RADIO__abmRadio[1]
#endif
/* Define default image check */
#ifndef RADIO_IMAGE_CHECK_DEFAULT
#define RADIO_IMAGE_CHECK_DEFAULT   &RADIO__bmCheck
#endif
/* Define default font */
#ifndef RADIO_FONT_DEFAULT
#define RADIO_FONT_DEFAULT          &GUI_Font13_1
#endif
/* Define default text color */
#ifndef RADIO_DEFAULT_TEXT_COLOR
#define RADIO_DEFAULT_TEXT_COLOR    GUI_BLACK
#endif
/* Define default background color */
#ifndef RADIO_DEFAULT_BKCOLOR
#define RADIO_DEFAULT_BKCOLOR       0xC0C0C0
#endif
#define RADIO_BORDER                  2
tRADIO_SetValue* RADIO__pfHandleSetValue;
RGB_COLOR         RADIO__DefaultTextColor       = RADIO_DEFAULT_TEXT_COLOR;
const GUI_FONT GUI_UNI_PTR* RADIO__pDefaultFont = RADIO_FONT_DEFAULT;
const GUI_BITMAP* RADIO__apDefaultImage[]       = {RADIO_IMAGE0_DEFAULT, RADIO_IMAGE1_DEFAULT};
const GUI_BITMAP* RADIO__pDefaultImageCheck     = RADIO_IMAGE_CHECK_DEFAULT;

static void _ResizeRect(GUI_RECT* pDest, const GUI_RECT* pSrc, int Diff) {
  pDest->y0 = pSrc->y0 - Diff;
  pDest->y1 = pSrc->y1 + Diff;
  pDest->x0 = pSrc->x0 - Diff;
  pDest->x1 = pSrc->x1 + Diff;
}
/*********************************************************************
*
*       _OnPaint
*
* Purpose:
*   Paints the RADIO button.
*   The button can actually consist of multiple buttons (NumItems).
*   The focus rectangle will be drawn on top of the text if any text is set,
*   otherwise around the entire buttons.
*/
static void _OnPaint(RADIO_Handle hObj, RADIO_Obj* pObj) {
  const GUI_BITMAP* pBmRadio;
  const GUI_BITMAP* pBmCheck;
  const char* pText;
  GUI_FONTINFO FontInfo;
  GUI_RECT Rect, r, rFocus = {0};
  int i, y, HasFocus, FontDistY;
  uint8_t SpaceAbove, CHeight, FocusBorder;
  /* Init some data */
  WIDGET__GetClientRect(&pObj->Widget, &rFocus);
  HasFocus  = (pObj->Widget.State & WIDGET_STATE_FOCUS) ? 1 : 0;
  pBmRadio  = pObj->apBmRadio[WM__IsEnabled(hObj)];
  pBmCheck  = pObj->pBmCheck;
  rFocus.x1 = pBmRadio->XSize + RADIO_BORDER * 2 - 1;
  rFocus.y1 = pObj->Height + ((pObj->NumItems - 1) * pObj->Spacing) - 1;
  /* Select font and text color */
  LCD_SetColor(pObj->TextColor);
  GUI_SetFont(pObj->pFont);
  GUI_SetTextMode(GUI_TM_TRANS);
  /* Get font infos */
  GUI_GetFontInfo(pObj->pFont, &FontInfo);
  FontDistY   = GUI_GetFontDistY();
  CHeight     = FontInfo.CHeight;
  SpaceAbove  = FontInfo.Baseline - CHeight;
  Rect.x0     = pBmRadio->XSize + RADIO_BORDER * 2 + 2;
  Rect.y0     = (CHeight <= pObj->Height) ? ((pObj->Height - CHeight) / 2) : 0;
  Rect.y1     = Rect.y0 + CHeight - 1;
  FocusBorder = (FontDistY <= 12) ? 2 : 3;
  if (Rect.y0 < FocusBorder) {
    FocusBorder = Rect.y0;
  }
  /* Clear inside ... Just in case      */
  /* Fill with parents background color */
#if WM_SUPPORT_TRANSPARENCY
  if (!WM_GetHasTrans(hObj))
#endif
  {
    if (pObj->BkColor != GUI_INVALID_COLOR) {
      LCD_SetBkColor(pObj->BkColor);
    } else {
      LCD_SetBkColor(RADIO_DEFAULT_BKCOLOR);
    }
    GUI_Clear();
  }
  /* Iterate over all items */
  for (i = 0; i < pObj->NumItems; i++) {
    y = i * pObj->Spacing;
    /* Draw the radio button bitmap */
    GUI_DrawBitmap(pBmRadio, RADIO_BORDER, RADIO_BORDER + y);
    /* Draw the check bitmap */
    if (pObj->Sel == i) {
      GUI_DrawBitmap(pBmCheck, RADIO_BORDER +  (pBmRadio->XSize - pBmCheck->XSize) / 2,
                               RADIO_BORDER + ((pBmRadio->YSize - pBmCheck->YSize) / 2) + y);
    }
    /* Draw text if available */
    pText = (const char*)GUI_ARRAY_GetpItem(&pObj->TextArray, i);
    if (pText) {
      if (*pText) {
        r = Rect;
        r.x1 = r.x0 + GUI_GetStringDistX(pText) - 2;
        GUI_MoveRect(&r, 0, y);
        GUI_DispStringAt(pText, r.x0, r.y0 - SpaceAbove);
        /* Calculate focus rect */
        if (HasFocus && (pObj->Sel == i)) {
          _ResizeRect(&rFocus, &r, FocusBorder);
        }
      }
    }
  }
  /* Draw the focus rect */
  if (HasFocus) {
    LCD_SetColor(GUI_BLACK);
    WIDGET__DrawFocusRect(&pObj->Widget, &rFocus, 0);
  }
}
static void _OnTouch(RADIO_Handle hObj, RADIO_Obj* pObj, WM_MESSAGE*pMsg) {
  int Notification;
  int Hit = 0;
  GUI_PID_STATE* pState = (GUI_PID_STATE*)pMsg->Data.p;
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    if (pState->Pressed) {
      int y, Sel;
      y   = pState->y;
      Sel = y   / pObj->Spacing;
      y  -= Sel * pObj->Spacing;
      if (y <= pObj->Height) {
        RADIO_SetValue(hObj, Sel);
      }
      if (WM_IsFocussable(hObj)) {
        WM_SetFocus(hObj);
      }
      Notification = WM_NOTIFICATION_CLICKED;
    } else {
      Hit = 1;
      Notification = WM_NOTIFICATION_RELEASED;
    }
  } else {
    Notification = WM_NOTIFICATION_MOVED_OUT;
  }
  WM_NotifyParent(hObj, Notification);
  if (Hit == 1) {
    GUI_StoreKey(pObj->Widget.Id);
  }
}
static void  _OnKey(RADIO_Handle hObj, WM_MESSAGE* pMsg) {
  WM_KEY_INFO* pKeyInfo;
  pKeyInfo = (WM_KEY_INFO*)(pMsg->Data.p);
  if (pKeyInfo->PressedCnt > 0) {
    switch (pKeyInfo->Key) {
    case GUI_KEY_RIGHT:
    case GUI_KEY_DOWN:
      RADIO_Inc(hObj);
      break;                    /* Send to parent by not doing anything */
    case GUI_KEY_LEFT:
    case GUI_KEY_UP:
      RADIO_Dec(hObj);
      break;                    /* Send to parent by not doing anything */
    default:
      return;
    }
  }
}
static void _RADIO_Callback (WM_MESSAGE* pMsg) {
  RADIO_Handle hObj;
  RADIO_Obj*   pObj;
  hObj = pMsg->hWin;
  pObj = (hObj);
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_PAINT:
    _OnPaint(hObj, pObj);
    return;
  case WM_GET_RADIOGROUP:
    pMsg->Data.v = pObj->GroupId;
    return;
  case WM_TOUCH:
    _OnTouch(hObj, pObj, pMsg);
    break;
  case WM_KEY:
    _OnKey(hObj, pMsg);
    break;
  case WM_DELETE:
    GUI_ARRAY_Delete(&pObj->TextArray);
    break;
  }
  WM_DefaultProc(pMsg);
}
void RADIO__SetValue(RADIO_Handle hObj, RADIO_Obj* pObj, int v) {
  if (v >= pObj->NumItems) {
    v = (int)pObj->NumItems - 1;
  }
  if (v != pObj->Sel) {
    pObj->Sel = v;
    WM_InvalidateWindow(hObj);
    WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
  }
}
/* Note: the parameters to a create function may vary.
         Some widgets may have multiple create functions */
RADIO_Handle RADIO_CreateEx(int x0, int y0, int xSize, int ySize, WM_HWIN hParent,
                            int WinFlags, int ExFlags, int Id, int NumItems, int Spacing)
{
  RADIO_Handle hObj;
  int Height, i;
  /* Calculate helper variables */
  Height   = RADIO__apDefaultImage[0]->YSize + RADIO_BORDER * 2;
  Spacing  = (Spacing  <= 0) ? 20 : Spacing;
  NumItems = (NumItems <= 0) ?  2 : NumItems;
  if (ySize == 0) {
    ySize  = Height + ((NumItems - 1) * Spacing);
  }
  if (xSize == 0) {
    xSize  = RADIO__apDefaultImage[0]->XSize + RADIO_BORDER * 2;
  }
#if WM_SUPPORT_TRANSPARENCY
  WinFlags |= WM_CF_HASTRANS;
#endif
  /* Create the window */
  hObj = WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent, WinFlags, _RADIO_Callback, sizeof(RADIO_Obj) - sizeof(WM_Obj));
  if (hObj) {
    RADIO_Obj* pObj;

    pObj = (hObj);
    /* Init sub-classes */
    GUI_ARRAY_CREATE(&pObj->TextArray);
    for (i = 0; i < NumItems; i++) {
      GUI_ARRAY_AddItem(&pObj->TextArray, NULL, 0);
    }
    /* Init widget specific variables */
    ExFlags &= RADIO_TEXTPOS_LEFT;
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE | ExFlags);
    /* Init member variables */
    pObj->apBmRadio[0] = RADIO__apDefaultImage[0];
    pObj->apBmRadio[1] = RADIO__apDefaultImage[1];
    pObj->pBmCheck     = RADIO__pDefaultImageCheck;
    pObj->pFont        = RADIO__pDefaultFont;
    pObj->TextColor    = RADIO__DefaultTextColor;
    pObj->BkColor      = WM_GetBkColor(hParent);
    pObj->NumItems     = NumItems;
    pObj->Spacing      = Spacing;
    pObj->Height       = Height;

  } else {
  }
  return hObj;
}
void RADIO_AddValue(RADIO_Handle hObj, int Add) {
  if (hObj) {
    RADIO_Obj* pObj;

    pObj = (hObj);
    RADIO_SetValue(hObj, pObj->Sel + Add);

  }
}
void RADIO_Dec(RADIO_Handle hObj) {
  RADIO_AddValue(hObj, -1);
}
void RADIO_Inc(RADIO_Handle hObj) {
  RADIO_AddValue(hObj,  1);
}
void RADIO_SetValue(RADIO_Handle hObj, int v) {
  if (hObj) {
    RADIO_Obj* pObj;

    pObj = (hObj);
    if (pObj->GroupId && RADIO__pfHandleSetValue) {
      (*RADIO__pfHandleSetValue)(hObj, pObj, v);
    } else {
      if (v < 0) {
        v = 0;
      }
      RADIO__SetValue(hObj, pObj, v);
    }

  }
}
int RADIO_GetValue(RADIO_Handle hObj) {
  int r = 0;
  if (hObj) {
    RADIO_Obj* pObj;

    pObj = (hObj);
    r = pObj->Sel;

  }
  return r;
}

RADIO_Handle RADIO_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, unsigned Para) {
	return RADIO_CreateEx(x0, y0, xsize, ysize, hParent, Flags, 0, Id, Para & 0xFF, (Para >> 8) & 0xFF);
}

RADIO_Handle RADIO_CreateIndirect(const GUI_WIDGET_CREATE_INFO *pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK *cb) {
	RADIO_Handle  hThis;
	int NumItems = (pCreateInfo->Para) & 0xFF;
	int Spacing = (pCreateInfo->Para >> 8) & 0xFF;
	GUI_USE_PARA(cb);
	hThis = RADIO_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize,
						   hWinParent, pCreateInfo->Flags, 0, pCreateInfo->Id, NumItems, Spacing);
	return hThis;
}

const GUI_FONT GUI_UNI_PTR *RADIO_GetDefaultFont(void) {
	return RADIO__pDefaultFont;
}
RGB_COLOR RADIO_GetDefaultTextColor(void) {
	return RADIO__DefaultTextColor;
}
void RADIO_SetDefaultFont(const GUI_FONT GUI_UNI_PTR *pFont) {
	RADIO__pDefaultFont = pFont;
}
void RADIO_SetDefaultTextColor(RGB_COLOR TextColor) {
	RADIO__DefaultTextColor = TextColor;
}


void RADIO_SetBkColor(RADIO_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		RADIO_Obj *pObj;

		pObj = (hObj);
		if (Color != pObj->BkColor) {
			pObj->BkColor = Color;
#if WM_SUPPORT_TRANSPARENCY
			if (Color <= 0xFFFFFF) {
				WM_SetTransState(hObj, 0);
			}
			else {
				WM_SetTransState(hObj, WM_CF_HASTRANS);
			}
#endif
			WM_InvalidateWindow(hObj);
		}

	}
}

void RADIO_SetDefaultImage(const GUI_BITMAP *pBitmap, unsigned int Index) {
	switch (Index) {
		case RADIO_BI_INACTIV:
		case RADIO_BI_ACTIV:
			RADIO__apDefaultImage[Index] = pBitmap;
			break;
		case RADIO_BI_CHECK:
			RADIO__pDefaultImageCheck = pBitmap;
			break;
	}
}

void RADIO_SetFont(RADIO_Handle hObj, const GUI_FONT GUI_UNI_PTR *pFont) {
	if (hObj) {
		RADIO_Obj *pObj;

		pObj = (hObj);
		if (pFont != pObj->pFont) {
			pObj->pFont = pFont;
			if (GUI_ARRAY_GetNumItems(&pObj->TextArray)) {
				WM_InvalidateWindow(hObj);
			}
		}

	}
}

static void _SetValue(RADIO_Handle hObj, int v) {
	RADIO_Obj *pObj;
	pObj = (hObj);
	RADIO__SetValue(hObj, pObj, v);
}
static int _IsInGroup(WM_HWIN hWin, uint8_t GroupId) {
	if (GroupId) {
		WM_MESSAGE Msg;
		Msg.MsgId = WM_GET_RADIOGROUP;
		WM_SendMessage(hWin, &Msg);
		return (Msg.Data.v == GroupId);
	}
	return 0;
}
static WM_HWIN _GetPrevInGroup(WM_HWIN hWin, uint8_t GroupId) {
	for (hWin = WM__GetPrevSibling(hWin); hWin; hWin = WM__GetPrevSibling(hWin)) {
		if (_IsInGroup(hWin, GroupId)) {
			return hWin;
		}
	}
	return 0;
}
static WM_HWIN _GetNextInGroup(WM_HWIN hWin, uint8_t GroupId) {
	for (; hWin; hWin = WM_GetNextSibling(hWin)) {
		if (_IsInGroup(hWin, GroupId)) {
			return hWin;
		}
	}
	return 0;
}
static void _ClearSelection(RADIO_Handle hObj, uint8_t GroupId) {
	WM_HWIN hWin;
	WM_Obj *pWin;
	for (hWin = WM__GetFirstSibling(hObj); hWin; hWin = pWin->hNext) {
		pWin = (hWin);
		if (hWin != hObj) {
			if (_IsInGroup(hWin, GroupId)) {
				RADIO__SetValue(hWin, (RADIO_Obj *)pWin, -1);
			}
		}
	}
}
static void _HandleSetValue(RADIO_Handle hObj, RADIO_Obj *pObj, int v) {
	if (v < 0) {
		WM_HWIN hWin = _GetPrevInGroup(hObj, pObj->GroupId);
		if (hWin) {
			WM_SetFocus(hWin);
			_SetValue(hWin, 0x7FFF);
			RADIO__SetValue(hObj, pObj, -1);
		}
	}
	else if (v >= pObj->NumItems) {
		WM_HWIN hWin = _GetNextInGroup(pObj->Widget.Win.hNext, pObj->GroupId);
		if (hWin) {
			WM_SetFocus(hWin);
			_SetValue(hWin, 0);
			RADIO__SetValue(hObj, pObj, -1);
		}
	}
	else {
		if (pObj->Sel != v) {
			_ClearSelection(hObj, pObj->GroupId);
			RADIO__SetValue(hObj, pObj, v);
		}
	}
}

void RADIO_SetGroupId(RADIO_Handle hObj, uint8_t NewGroupId) {
	if (hObj) {
		RADIO_Obj *pObj;
		uint8_t OldGroupId;
		pObj = (hObj);
		OldGroupId = pObj->GroupId;
		if (NewGroupId != OldGroupId) {
			WM_HWIN hFirst;
			hFirst = WM__GetFirstSibling(hObj);
			/* Set function pointer if necessary */
			if (NewGroupId && (RADIO__pfHandleSetValue == NULL)) {
				RADIO__pfHandleSetValue = _HandleSetValue;
			}
			/* Pass our selection, if we have one, to another radio button in */
			/* our old group. So the group have a valid selection when we leave it. */
			if (OldGroupId && (pObj->Sel >= 0)) {
				WM_HWIN hWin;
				pObj->GroupId = 0; /* Leave group first, so _GetNextInGroup() could */
				/* not find a handle to our own window. */
				hWin = _GetNextInGroup(hFirst, OldGroupId);
				if (hWin) {
					_SetValue(hWin, 0);
				}
			}
			/* Make sure we have a valid selection according to our new group */
			if (_GetNextInGroup(hFirst, NewGroupId) != 0) {
				/* Join an existing group with an already valid selection, so clear our own one */
				RADIO__SetValue(hObj, pObj, -1);
			}
			else if (pObj->Sel < 0) {
				/* We are the first window in group, so we must have a valid selection at our own. */
				RADIO__SetValue(hObj, pObj, 0);
			}
			/* Change the group */
			pObj->GroupId = NewGroupId;
		}
	}
}


void RADIO_SetImage(RADIO_Handle hObj, const GUI_BITMAP *pBitmap, unsigned int Index) {
	if (hObj) {
		RADIO_Obj *pObj;

		pObj = (hObj);
		switch (Index) {
			case RADIO_BI_INACTIV:
			case RADIO_BI_ACTIV:
				pObj->apBmRadio[Index] = pBitmap;
				break;
			case RADIO_BI_CHECK:
				pObj->pBmCheck = pBitmap;
				break;
		}
		WM_InvalidateWindow(hObj);

	}
}

void RADIO_SetText(RADIO_Handle hObj, const char *pText, unsigned Index) {
	if (hObj) {
		RADIO_Obj *pObj;

		pObj = (hObj);
		if (Index < (unsigned)pObj->NumItems) {
			GUI_ARRAY_SetItem(&pObj->TextArray, Index, pText, pText ? (GUI__strlen(pText) + 1) : 0);
			WM_InvalidateWindow(hObj);
		}

	}
}

void RADIO_SetTextColor(RADIO_Handle hObj, RGB_COLOR Color) {
	if (hObj) {
		RADIO_Obj *pObj;

		pObj = (hObj);
		if (Color != pObj->TextColor) {
			pObj->TextColor = Color;
			if (GUI_ARRAY_GetNumItems(&pObj->TextArray)) {
				WM_InvalidateWindow(hObj);
			}
		}

	}
}

#ifndef RADIO_BKCOLOR0_DEFAULT
#define RADIO_BKCOLOR0_DEFAULT 0xc0c0c0           /* Inactive color */
#endif
#ifndef RADIO_BKCOLOR1_DEFAULT
#define RADIO_BKCOLOR1_DEFAULT GUI_WHITE          /* Active color */
#endif


/* Colors */
static const RGB_COLOR _aColorDisabled[] = { 0xC0C0C0, 0x808080, 0x000000, RADIO_BKCOLOR0_DEFAULT };
static const RGB_COLOR _aColorEnabled[] = { 0xC0C0C0, 0x808080, 0x000000, RADIO_BKCOLOR1_DEFAULT };
static const RGB_COLOR _ColorsCheck[] = { 0xFFFFFF, 0x000000 };
/* Palettes */
static const GUI_LOGPALETTE _PalRadioDisabled = {
  4,	/* number of entries */
  1, 	/* Transparency */
  _aColorDisabled
};
static const GUI_LOGPALETTE _PalRadioEnabled = {
  4,	/* number of entries */
  1, 	/* Transparency */
  _aColorEnabled
};
static const GUI_LOGPALETTE _PalCheck = {
  2,	/* number of entries */
  1, 	/* Transparency */
  &_ColorsCheck[0]
};

/* Pixel data */
static const unsigned char _acRadio[] = {
  0x00, 0x55, 0x00,
  0x05, 0xAA, 0x50,
  0x1A, 0xFF, 0xAC,
  0x1B, 0xFF, 0xCC,
  0x6F, 0xFF, 0xF3,
  0x6F, 0xFF, 0xF3,
  0x6F, 0xFF, 0xF3,
  0x6F, 0xFF, 0xF3,
  0x1B, 0xFF, 0xCC,
  0x10, 0xFF, 0x0C,
  0x0F, 0x00, 0xF0,
  0x00, 0xFF, 0x00
};
static const unsigned char _acCheck[] = {
  _XX_____,
  XXXX____,
  XXXX____,
  _XX_____
};
/* Bitmaps */
const GUI_BITMAP RADIO__abmRadio[] = {
  { 12, 12, 3, 2, _acRadio, &_PalRadioDisabled},
  { 12, 12, 3, 2, _acRadio, &_PalRadioEnabled}
};

const GUI_BITMAP RADIO__bmCheck = {
  4, /* XSize */
  4, /* YSize */
  1, /* BytesPerLine */
  1, /* BitsPerPixel */
  _acCheck,  /* Pointer to picture data (indices) */
  &_PalCheck  /* Pointer to palette */
};
