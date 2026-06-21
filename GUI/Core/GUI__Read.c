

#include "GUI_Protected.h"

U16 GUI__Read16(const U8 ** ppData) {
  const U8 * pData;
  U16  Value;
  pData = *ppData;
  Value = *pData;
  Value |= (U16)(*(pData + 1) << 8);
  pData += 2;
  *ppData = pData;
  return Value;
}

U32 GUI__Read32(const U8 ** ppData) {
  const U8 * pData;
  U32  Value;
  pData = *ppData;
  Value = *pData;
  Value |= (     *(pData + 1) << 8);
  Value |= ((U32)*(pData + 2) << 16);
  Value |= ((U32)*(pData + 3) << 24);
  pData += 4;
  *ppData = pData;
  return Value;
}

