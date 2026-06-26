

#include "GUI_Private.h"

int GUI_GetBitmapPixelIndex(const GUI_BITMAP  * pBMP, unsigned x, unsigned y) {
  unsigned Off, Value;
  switch (pBMP->BitsPerPixel) {
  case 1:
    Off = (x >> 3) + (y * pBMP->BytesPerLine);
    Value = *(pBMP->pData + Off);
    Value = Value >> (7 - (x & 0x7)) & 0x1;
    break;
  case 2:
    Off = (x >> 2) + (y * pBMP->BytesPerLine);
    Value = *(pBMP->pData + Off);
    Value = Value >> (6 - ((x << 1) & 0x6)) & 0x3;
    break;
  case 4:
    Off = (x >> 1) + (y * pBMP->BytesPerLine);
    Value = *(pBMP->pData + Off);
    Value = (x & 1) ? (Value & 0xF) : (Value >> 4);
    break;
  case 8:
    Off = x + y * pBMP->BytesPerLine;
    Value = *(pBMP->pData + Off);
    break;
  case 16:
    Off = (x << 1) + y * pBMP->BytesPerLine;
    Value = *(pBMP->pData + Off) | (*(pBMP->pData + Off + 1) << 8);
    break;
  default:
    Value = 0;
  }
  return Value;
}


