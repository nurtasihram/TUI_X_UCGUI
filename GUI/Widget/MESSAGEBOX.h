#pragma once

#include "WM.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

WM_HWIN MESSAGEBOX_Create(const char * sMessage, const char * sCaption, int Flags);

#if defined(__cplusplus)
  }
#endif
