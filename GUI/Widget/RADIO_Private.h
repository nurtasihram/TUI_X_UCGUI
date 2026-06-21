#pragma once
#include "WM.h"
#include "RADIO.h"
#include "WIDGET.h"
#include "GUI_ARRAY.h"
typedef struct {
  WIDGET Widget;
  const GUI_BITMAP* apBmRadio[2];
  const GUI_BITMAP* pBmCheck;
  GUI_ARRAY TextArray;
  int16_t Sel;                   /* current selection */
  uint16_t Spacing;
  uint16_t Height;
  uint16_t NumItems;
  uint8_t  GroupId;
  RGB_COLOR BkColor;
  RGB_COLOR TextColor;
  const GUI_FONT GUI_UNI_PTR* pFont;
#if GUI_DEBUG_LEVEL >1
    int DebugId;
#endif
} RADIO_Obj;
#define RADIO_H2P(h) (RADIO_Obj*) GUI_ALLOC_h2p(h)
typedef void tRADIO_SetValue(RADIO_Handle hObj, RADIO_Obj* pObj, int v);
extern const GUI_BITMAP             RADIO__abmRadio[2];
extern const GUI_BITMAP             RADIO__bmCheck;
extern const GUI_BITMAP*            RADIO__apDefaultImage[2];
extern const GUI_BITMAP*            RADIO__pDefaultImageCheck;
extern const GUI_FONT GUI_UNI_PTR*  RADIO__pDefaultFont;
extern       RGB_COLOR              RADIO__DefaultTextColor;
extern       tRADIO_SetValue*       RADIO__pfHandleSetValue;
void RADIO__SetValue(RADIO_Handle hObj, RADIO_Obj* pObj, int v);

