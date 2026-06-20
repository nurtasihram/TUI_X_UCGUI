

#ifndef  GUI_CONFDEFAULTS_H
#define  GUI_CONFDEFAULTS_H

#include "GUIConf.h"

/**********************************************************************
*
*       Defaults for config switches
*
***********************************************************************
*/

/* Define "universal pointer". Normally, this is not needed (define will expand to nothing)
   However, on some systems (AVR - IAR compiler) it can be necessary ( -> __generic),
   since a default pointer can access RAM only, not the built-in Flash
*/
#ifndef GUI_UNI_PTR
  #define GUI_UNI_PTR
  #define GUI_UNI_PTR_USED 0
#else
  #define GUI_UNI_PTR_USED 1
#endif

/* Define const storage. Normally, this is not needed (define will expand to const)
   However, on some systems (AVR - IAR compiler) it can be necessary ( -> __flash const),
   since otherwise constants are copied into RAM
*/
#ifndef GUI_ALLOC_SIZE
  #define GUI_ALLOC_SIZE      1000
#endif

#ifndef GUI_CONST_STORAGE
  #define GUI_CONST_STORAGE const
#endif

#ifndef GUI_SUPPORT_TOUCH
  #define GUI_SUPPORT_TOUCH   0
#endif

#ifndef GUI_SUPPORT_MOUSE
  #define GUI_SUPPORT_MOUSE   0
#endif

#ifndef GUI_SUPPORT_MEMDEV
  #define GUI_SUPPORT_MEMDEV  0
#endif

#ifndef GUI_SUPPORT_CURSOR
  #define GUI_SUPPORT_CURSOR  (GUI_SUPPORT_MOUSE | GUI_SUPPORT_TOUCH)
#endif

#ifndef GUI_SUPPORT_DEVICES
  #ifdef __C51__               /* Keil C51 limitation ... Indirect function calls are limited */
    #define GUI_SUPPORT_DEVICES 0
  #else
    #define GUI_SUPPORT_DEVICES (GUI_SUPPORT_MEMDEV)
  #endif
#endif

/* Some compilers for very simple chips can not generate code for function pointers
with parameters. In this case, we do not use function pointers, but limit the functionality slightly */
#ifndef GUI_COMPILER_SUPPORTS_FP
  #ifdef __C51__
    #define GUI_COMPILER_SUPPORTS_FP 0
  #else
    #define GUI_COMPILER_SUPPORTS_FP 1
  #endif
#endif

/* In order to avoid warnings for undefined parameters */
#ifndef GUI_USE_PARA
  #if defined (__BORLANDC__) || defined(NC30) || defined(NC308)
    #define GUI_USE_PARA(para)
  #else
    #define GUI_USE_PARA(para) para=para;
  #endif
#endif

/* Default for types */
#ifndef GUI_TIMER_TIME
  #define GUI_TIMER_TIME int  /* default is to use 16 bits for 16 bit CPUs,
	                           32 bits on 32 bit CPUs for timing */
#endif

/* Types used for memory allocation */
#if GUI_ALLOC_SIZE <32767
  #define GUI_ALLOC_DATATYPE   I16
  #define GUI_ALLOC_DATATYPE_U U16
#else
  #define GUI_ALLOC_DATATYPE   I32
  #define GUI_ALLOC_DATATYPE_U U32
#endif

#ifndef   GUI_MEMSET
  #define GUI_MEMSET GUI__memset
#endif


#endif   /* ifdef GUI_CONFDEFAULTS_H */
/*************************** End of file ****************************/
