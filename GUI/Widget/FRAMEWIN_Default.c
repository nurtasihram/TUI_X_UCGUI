
#include "GUI_Protected.h"
#include "FRAMEWIN_Private.h"

void FRAMEWIN_SetDefaultFont(const GUI_FONT GUI_UNI_PTR * pFont) {
  FRAMEWIN__DefaultProps.pFont = pFont;
}
const GUI_FONT GUI_UNI_PTR * FRAMEWIN_GetDefaultFont(void) {
  return FRAMEWIN__DefaultProps.pFont;
}
void FRAMEWIN_SetDefaultBarColor(unsigned Index, GUI_COLOR Color) {
  if (Index < GUI_COUNTOF(FRAMEWIN__DefaultProps.aBarColor)) {
    FRAMEWIN__DefaultProps.aBarColor[Index] = Color;
  }
}
GUI_COLOR FRAMEWIN_GetDefaultBarColor(unsigned Index) {
  GUI_COLOR Color = 0;
  if (Index < GUI_COUNTOF(FRAMEWIN__DefaultProps.aBarColor)) {
    Color = FRAMEWIN__DefaultProps.aBarColor[Index];
  }
  return Color;
}
void FRAMEWIN_SetDefaultClientColor(GUI_COLOR Color) {
  FRAMEWIN__DefaultProps.ClientColor = Color;
}
GUI_COLOR FRAMEWIN_GetDefaultClientColor(void) {
  return FRAMEWIN__DefaultProps.ClientColor;
}
void FRAMEWIN_SetDefaultTitleHeight(int Height) {
  FRAMEWIN__DefaultProps.TitleHeight = Height;
}
int FRAMEWIN_GetDefaultTitleHeight(void) {
  return FRAMEWIN__DefaultProps.TitleHeight;
}
void FRAMEWIN_SetDefaultBorderSize(int DefaultBorderSize) {
  FRAMEWIN__DefaultProps.BorderSize = DefaultBorderSize;
}
int FRAMEWIN_GetDefaultBorderSize(void) {
  return FRAMEWIN__DefaultProps.BorderSize;
}
void FRAMEWIN_SetDefaultTextColor(unsigned Index, GUI_COLOR Color) {
  if (Index < GUI_COUNTOF(FRAMEWIN__DefaultProps.aTextColor)) {
    FRAMEWIN__DefaultProps.aTextColor[Index] = Color;
  }
}
GUI_COLOR FRAMEWIN_GetDefaultTextColor(unsigned Index) {
  GUI_COLOR Color = 0;
  if (Index < GUI_COUNTOF(FRAMEWIN__DefaultProps.aTextColor)) {
    Color = FRAMEWIN__DefaultProps.aTextColor[Index];
  }
  return Color;
}
#else
  void FRAMEWIN_Default(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
