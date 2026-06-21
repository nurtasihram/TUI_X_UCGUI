#pragma once

#include "GUI.h"
#include "GUI_X.h"

#define GUI_DEBUG_LEVEL_NOCHECK       0  /* No run time checks are performed */
#define GUI_DEBUG_LEVEL_CHECK_PARA    1  /* Parameter checks are performed to avoid crashes */
#define GUI_DEBUG_LEVEL_CHECK_ALL     2  /* Parameter checks and consistency checks are performed */
#define GUI_DEBUG_LEVEL_LOG_ERRORS    3  /* Errors are recorded */
#define GUI_DEBUG_LEVEL_LOG_WARNINGS  4  /* Errors & Warnings are recorded */
#define GUI_DEBUG_LEVEL_LOG_ALL       5  /* Errors, Warnings and Messages are recorded. */

#ifndef GUI_DEBUG_LEVEL
#define GUI_DEBUG_LEVEL GUI_DEBUG_LEVEL_LOG_ALL  /* Simulation should log all warnings */
#endif

/* Make sure the macros are actually defined */

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_LOG_ERRORS
#define GUI_DEBUG_ERROROUT(s, ...)        GUI_X_ErrorOut(s, ##__VA_ARGS__)
#define GUI_DEBUG_ERROROUT_IF(exp,s, ...) { if (exp) GUI_DEBUG_ERROROUT(s, ##__VA_ARGS__); }
#else
#define GUI_DEBUG_ERROROUT(s, ...)
#define GUI_DEBUG_ERROROUT_IF(exp,s, ...)
#endif

/* Make sure the macros are actually defined */

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_LOG_WARNINGS
#define GUI_DEBUG_WARN(s, ...)        GUI_X_Warn(s, ##__VA_ARGS__)
#define GUI_DEBUG_WARN_IF(exp,s, ...) { if (exp) GUI_DEBUG_WARN(s, ##__VA_ARGS__); }
#else
#define GUI_DEBUG_WARN(s, ...)
#define GUI_DEBUG_WARN_IF(exp,s, ...)
#endif

/* Make sure the macros are actually defined */

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_LOG_ALL
#define GUI_DEBUG_LOG(s, ...)        GUI_X_Log(s, ##__VA_ARGS__)
#define GUI_DEBUG_LOG_IF(exp,s, ...) { if (exp) GUI_DEBUG_LOG(s, ##__VA_ARGS__); }
#else
#define GUI_DEBUG_LOG(s, ...)
#define GUI_DEBUG_LOG_IF(exp,s, ...)
#endif
