#pragma once
#include "SCROLLBAR.h"
#include "WIDGET.h"
#include "GUIDebug.h"
#define SCROLLBAR_ID 0x4544   /* Magic numer, should be unique if possible */
#define SCROLLBAR_H2P(h) (SCROLLBAR_Obj*) GUI_ALLOC_h2p(h)
#if GUI_DEBUG_LEVEL > 1
#define SCROLLBAR_ASSERT_IS_VALID_PTR(p) DEBUG_ERROROUT_IF(p->DebugId != SCROLLBAR_ID, "xxx.c: Wrong handle type or Object not init'ed")
#define SCROLLBAR_INIT_ID(p)   p->DebugId = SCROLLBAR_ID
#define SCROLLBAR_DEINIT_ID(p) p->DebugId = SCROLLBAR_ID+1
#else
#define SCROLLBAR_ASSERT_IS_VALID_PTR(p)
#define SCROLLBAR_INIT_ID(p)
#define SCROLLBAR_DEINIT_ID(p)
#endif
extern RGB_COLOR  SCROLLBAR__aDefaultBkColor[2];
extern RGB_COLOR  SCROLLBAR__aDefaultColor[2];
extern int16_t        SCROLLBAR__DefaultWidth;
typedef struct {
  WIDGET Widget;
  RGB_COLOR aBkColor[2];
  RGB_COLOR aColor[2];
  int NumItems, v, PageSize;
#if GUI_DEBUG_LEVEL >1
    int DebugId;
#endif
} SCROLLBAR_Obj;
typedef struct {
  int x0_LeftArrow;
  int x1_LeftArrow;
  int x0_Thumb;
  int x1_Thumb;
  int x0_RightArrow;
  int x1_RightArrow;
  int x1;
  int xSizeMoveable;
  int ThumbSize;
} SCROLLBAR_POSITIONS;
void SCROLLBAR__InvalidatePartner(SCROLLBAR_Handle hObj);
