#pragma once

#include "GUI_Protected.h"
#include "LCD_Private.h"

#if GUI_SUPPORT_DEVICES
#define LCDDEV_L0_DrawBitmap GUI.pDeviceAPI->pfDrawBitmap
#define LCDDEV_L0_DrawPixel  GUI.pDeviceAPI->pfDrawPixel
#define LCDDEV_L0_FillRect   GUI.pDeviceAPI->pfFillRect
#define LCDDEV_L0_GetPixel   GUI.pDeviceAPI->pfGetPixel
#define LCDDEV_L0_GetRect    GUI.pDeviceAPI->pfGetRect
#define LCDDEV_L0_GetPixel   GUI.pDeviceAPI->pfGetPixel
#define LCDDEV_L0_SetPixel   GUI.pDeviceAPI->pfSetPixel
#endif

#define GUI_POS_AUTO -4095   /* Position value for auto-pos */
