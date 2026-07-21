#pragma once

#include "GUI_ConfDefaults.h"
#include "GUIType.h"
#include "Dots.inl"


#define GUI_COUNTOF(a) (sizeof(a) / sizeof(a[0]))

/************************************************************
*
*                    GUI_KEY_...
*
*************************************************************
These ID values are basically meant to be used with widgets
Note that we have chosen the values to be close to existing
"standards", so do not change them unless forced to.

*/

#define GUI_KEY_BACKSPACE         8         /* ASCII: BACKSPACE Crtl-H */
#define GUI_KEY_TAB               9         /* ASCII: TAB       Crtl-I */
#define GUI_KEY_ENTER             13        /* ASCII: ENTER     Crtl-M */
#define GUI_KEY_LEFT              16
#define GUI_KEY_UP                17
#define GUI_KEY_RIGHT             18
#define GUI_KEY_DOWN              19
#define GUI_KEY_HOME              23
#define GUI_KEY_END               24
#define GUI_KEY_SHIFT             25
#define GUI_KEY_CONTROL           26
#define GUI_KEY_ESCAPE            27        /* ASCII: ESCAPE    0x1b   */
#define GUI_KEY_INSERT            29
#define GUI_KEY_DELETE            30
#define GUI_KEY_SPACE             32

#define GUI_KEY_F1                40
#define GUI_KEY_F2                41

/*********************************************************************
*
*       Dialog item IDs
*
*  The IDs below are arbitrary values. They just have to be unique
*  within the dialog.
*
*  If you use your own Ids, we recommend to use values above GUI_ID_USER.
*/

#define GUI_ID_OK                1
#define GUI_ID_CANCEL            2
#define GUI_ID_YES               3
#define GUI_ID_NO                4
#define GUI_ID_CLOSE             5
#define GUI_ID_HELP              6
#define GUI_ID_MAXIMIZE          7
#define GUI_ID_MINIMIZE          8

#define GUI_ID_VSCROLL  0xFE
#define GUI_ID_HSCROLL  0xFF

#define GUI_ID_USER     0x100

#define GUI_TS_NORMAL           (0)
#define GUI_TS_UNDERLINE        (1 << 0)
#define GUI_TS_STRIKETHRU       (1 << 1)
#define GUI_TS_OVERLINE         (1 << 2)

/*      *********************************
		*
		*      Standard colors
		*
		*********************************
*/
#pragma region Color definitions
constexpr RGBC COLOR_RGB (uint8_t r, uint8_t g, uint8_t b) { return (b) | ((g) << 8) | ((r) << 16); }
constexpr RGBC RGB_GRAYL (uint8_t a) { return COLOR_RGB(a,a,a); }
constexpr RGBC RGB_BLUEL (uint8_t a) { return COLOR_RGB(0,0,a); }
constexpr RGBC RGB_GREENL(uint8_t a) { return COLOR_RGB(0,a,0); }
constexpr RGBC RGB_REDL  (uint8_t a) { return COLOR_RGB(a,0,0); }
constexpr RGBC RGB_BLACK           = RGB_GRAYL(0x00);
constexpr RGBC RGB_DARKGRAY        = RGB_GRAYL(0x40);
constexpr RGBC RGB_GRAY            = RGB_GRAYL(0x80);
constexpr RGBC RGB_LIGHTGRAY       = RGB_GRAYL(0xD3);
constexpr RGBC RGB_WHITE           = RGB_GRAYL(0xFF);
constexpr RGBC RGB_BLUE            = COLOR_RGB(0x00,0x00,0xFF);
constexpr RGBC RGB_GREEN           = COLOR_RGB(0x00,0xFF,0x00);
constexpr RGBC RGB_RED             = COLOR_RGB(0xFF,0x00,0x00);
constexpr RGBC RGB_CYAN            = COLOR_RGB(0x00,0xFF,0xFF);
constexpr RGBC RGB_MAGENTA         = COLOR_RGB(0xFF,0x00,0xFF);
constexpr RGBC RGB_YELLOW          = COLOR_RGB(0xFF,0xFF,0x00);
constexpr RGBC RGB_LIGHTBLUE       = COLOR_RGB(0x80,0x80,0xFF);
constexpr RGBC RGB_LIGHTGREEN      = COLOR_RGB(0x80,0xFF,0x80);
constexpr RGBC RGB_LIGHTRED        = COLOR_RGB(0xFF,0x80,0x80);
constexpr RGBC RGB_LIGHTCYAN       = COLOR_RGB(0x80,0xFF,0xFF);
constexpr RGBC RGB_LIGHTMAGENTA    = COLOR_RGB(0xFF,0x80,0xFF);
constexpr RGBC RGB_LIGHTYELLOW     = COLOR_RGB(0xFF,0xFF,0x80);
constexpr RGBC RGB_DARKBLUE        = COLOR_RGB(0x00,0x00,0x80);
constexpr RGBC RGB_DARKGREEN       = COLOR_RGB(0x00,0x80,0x00);
constexpr RGBC RGB_DARKRED         = COLOR_RGB(0x80,0x00,0x00);
constexpr RGBC RGB_DARKCYAN        = COLOR_RGB(0x00,0x80,0x80);
constexpr RGBC RGB_DARKMAGENTA     = COLOR_RGB(0x80,0x00,0x80);
constexpr RGBC RGB_DARKYELLOW      = COLOR_RGB(0x80,0x80,0x00);
constexpr RGBC RGB_BROWN           = COLOR_RGB(0xA5,0x2A,0x2A);

constexpr RGBC RGB_INVALID_COLOR   = ~0;      /* Invalid color - more than 24 bits */
#pragma endregion

/*      *********************************
		*
		*      Coordinates
		*
		*********************************
*/
#define GUI_COORD_X 0
#define GUI_COORD_Y 1

/*********************************************************************
*
*            Standard Fonts
*
**********************************************************************

Note: The external declarations for the fonts are now also included in
		this file.
*/

/* Text alignment flags, horizontal */
#define TEXTALIGN_LEFT        (0<<0)
#define TEXTALIGN_RIGHT       (1<<0)
#define TEXTALIGN_CENTER      (2<<0)
#define TEXTALIGN_HORIZONTAL  (3<<0)
#define TEXTALIGN_HCENTER     TEXTALIGN_CENTER

/* Text alignment flags, vertical */
#define TEXTALIGN_TOP        (0<<2)
#define TEXTALIGN_BOTTOM     (1<<2)
#define TEXTALIGN_BASELINE   (2<<2)
#define TEXTALIGN_VCENTER    (3<<2)
#define TEXTALIGN_VERTICAL   TEXTALIGN_VCENTER

using TEXTALIGN = uint8_t;

/*    *********************************
	*
	*     Min/Max coordinates
	*
	*********************************
*/
/* Define minimum and maximum coordinates in x and y */
#define GUI_XMIN -4095
#define GUI_XMAX  4095
#define GUI_YMIN -4095
#define GUI_YMAX  4095

/*********************************************************************
*
*             GUI_CONTEXT
*
**********************************************************************

This structure is public for one reason only:
To allow the application to save and restore the context.
*/

typedef struct {
	/* Variables in LCD module */
	uint32_t aColor[2];
	GUI_RECT ClipRect;
	uint8_t DrawMode;
	/* Variables in GUICHAR module */
	PCFONT pAFont;
	const GUI_UC_ENC_APILIST *pUC_API; /* Unicode encoding API */
	GUI_POINT DispPos;
	int16_t TextMode, TextAlign;
	//RGBC Color, BkColor; /* Required only when changing devices and for speed opt (caching) */
	/* Variables in WM module */
	const GUI_RECT *WM__pUserClipRect;
	int xOff, yOff;
	/* Variables in MEMDEV module (with memory devices only) */
#if GUI_SUPPORT_DEVICES
	const tLCDDEV_APIList *pDeviceAPI;  /* function pointers only */
	GUI_HMEM    hDevData;
	GUI_RECT    ClipRectPrev;
#endif
} GUI_CONTEXT;

void         GUI_Init(void);
void         GUI_SetDefault(void);
GUI_DRAWMODE GUI_SetDrawMode(GUI_DRAWMODE dm);
void         GUI_SaveContext(GUI_CONTEXT *pContext);
void         GUI_RestoreContext(const GUI_CONTEXT *pContext);

int  GUI_RectsIntersect(const GUI_RECT *pr0, const GUI_RECT *pr1);

int  GUI__DivideRound(int a, int b);

RGBC GUI_GetBkColor(void);
RGBC GUI_GetColor(void);
void GUI_SetBkColor(RGBC);
void GUI_SetColor(RGBC);

int  GUI_BMP_Draw(const void *pFileData, int x0, int y0);
int  GUI_BMP_GetXSize(const void *pFileData);
int  GUI_BMP_GetYSize(const void *pFileData);

void GUI_Clear(void);
void GUI_ClearRect(GUI_RECT r);
void GUI_DrawFocusRect(GUI_RECT r, int Dist);
void GUI_DrawRect(GUI_RECT r);
void GUI_FillRect(GUI_RECT r);

void GUI_DrawBitmap(PCBITMAP pBM, int x0, int y0);

void GUI_DrawHLine(int y0, int x0, int x1);
void GUI_DrawVLine(int x0, int y0, int y1);
void GUI_DrawPixel(int x, int y);

#if GUI_SUPPORT_CURSOR
void               GUI_CURSOR_Activate(void);
void               GUI_CURSOR_Deactivate(void);
void               GUI_CURSOR_SetPosition(int x, int y);
PCCURSOR GUI_CURSOR_Select(PCCURSOR pCursor);
void               GUI_CURSOR_Show(void);
void               GUI_CURSOR_Hide(void);
void               GUI_CURSOR__TempShow(void);
bool               GUI_CURSOR__TempHide(const GUI_RECT *pRect);
#endif

void  GUI_DispChar(uint16_t c);
void  GUI_DispChars(uint16_t c, int Cnt);
void  GUI_DispCharAt(uint16_t c, int16_t x, int16_t y);
void  GUI_DispString(const char *s);
void  GUI_DispStringAt(const char *s, int x, int y);
void  GUI__DispStringInRect(const char *s, GUI_RECT *pRect, int TextAlign, int MaxNumChars);
void  GUI_DispStringInRect(const char *s, GUI_RECT *pRect, int Flags);
void  GUI_DispStringInRectMax(const char *s, GUI_RECT *pRect, int TextAlign, int MaxLen); /* Not to be doc. */
void  GUI_GetTextExtend(GUI_RECT *pRect, const char *s, int Len);
int   GUI_GetYAdjust(void);
PCFONT GUI_GetFont(void);
int   GUI_GetCharDistX(uint16_t c);
int   GUI_GetStringDistX(const char *s);
int   GUI_GetFontDistY(void);
int   GUI_GetFontSizeY(void);
int   GUI_GetYSizeOfFont(PCFONT pFont);
int   GUI_GetYDistOfFont(PCFONT pFont);
int   GUI_GetTextAlign(void);
int   GUI_GetTextMode(void);
char  GUI_IsInFont(PCFONT pFont, uint16_t c);
void  GUI_SetTextAlign(int Align);
void  GUI_SetTextMode(int Mode);
PCFONT GUI_SetFont(PCFONT pNewFont);
void  GUI_GotoXY(int x, int y);
void  GUI_DispNextLine(void);

int      GUI_UC_Encode(char *s, uint16_t Char);
int      GUI_UC_GetCharSize(const char *s);
uint16_t GUI_UC_GetCharCode(const char *s);
void     GUI_UC_SetEncodeNone(void);
void     GUI_UC_SetEncodeUTF8(void);

void GUI_UC_DispString(const uint16_t *s);

void *GUI_ALLOC_AllocInit(const void *pInitData, size_t Size);
void *GUI_ALLOC_AllocNoInit(size_t size);
void *GUI_ALLOC_AllocZero(size_t size);
void     GUI_ALLOC_Free(void *ptr);
void     GUI_ALLOC_FreePtr(void **pptr);
void *GUI_ALLOC_Realloc(void *ptr, size_t NewSize);
size_t   GUI_ALLOC_GetMaxSize(void);
void     GUI__memcpy(void *pDest, const void *pSrc, size_t NumBytes);
void     GUI__memmove(void *pDest, const void *pSrc, size_t NumBytes);

#define GUI_MEMDEV_HASTRANS       0
#define GUI_MEMDEV_NOTRANS    (1<<0)

#if GUI_SUPPORT_DEVICES
typedef void *GUI_MEMDEV_Handle;
typedef void GUI_CALLBACK_VOID_P(void *p);

/* Create a memory device which is compatible to the selected LCD */
GUI_MEMDEV_Handle GUI_MEMDEV_Create(int x0, int y0, int XSize, int YSize);
GUI_MEMDEV_Handle GUI_MEMDEV_CreateEx(int x0, int y0, int XSize, int YSize, int Flags);
GUI_MEMDEV_Handle GUI_MEMDEV_CreateFixed(int x0, int y0, int xsize, int ysize, int Flags,
											const tLCDDEV_APIList *pMemDevAPI);
void GUI_MEMDEV_Clear(GUI_MEMDEV_Handle hMem);
void GUI_MEMDEV_CopyFromLCD(GUI_MEMDEV_Handle hMem);
void GUI_MEMDEV_CopyToLCD(GUI_MEMDEV_Handle hMem);
void GUI_MEMDEV_CopyToLCDAt(GUI_MEMDEV_Handle hMem, int x, int y);
void GUI_MEMDEV_Delete(GUI_MEMDEV_Handle MemDev);
int  GUI_MEMDEV_GetXSize(GUI_MEMDEV_Handle hMem);
int  GUI_MEMDEV_GetYSize(GUI_MEMDEV_Handle hMem);
void GUI_MEMDEV_ReduceYSize(GUI_MEMDEV_Handle hMem, int YSize);
GUI_MEMDEV_Handle GUI_MEMDEV_Select(GUI_MEMDEV_Handle hMem);  /* Select (activate) a particular memory device. */
void  GUI_MEMDEV_SetOrg(GUI_MEMDEV_Handle hMem, int x0, int y0);
int   GUI_MEMDEV_Draw(GUI_RECT *pRect, GUI_CALLBACK_VOID_P *pfDraw, void *pData, int MemSize, int Flags);
#endif

void GUI_SelectLCD(void);

void GUI_Delay(int Period);
int  GUI_GetTime(void);
int  GUI_Exec(void);         /* Execute all jobs ... Return 0 if nothing was done. */
int  GUI_Exec1(void);        /* Execute one job  ... Return 0 if nothing was done. */

/*********************************************************************
*
*                 MessageBox
*
**********************************************************************

	Note: These should be moved into a separate file.
*/
int     GUI_MessageBox(const char *sMessage, const char *sCaption, int Flags);
#define GUI_MESSAGEBOX_CF_MOVEABLE (1<<4)

#define GUI_MB_OK                20
#define GUI_MB_WARNING           21

typedef struct {
	GUI_TIMER_TIME Time;
	uintptr_t Context;
} GUI_TIMER_MESSAGE;

typedef void *GUI_TIMER_HANDLE;
typedef void GUI_TIMER_CALLBACK(/*const*/ GUI_TIMER_MESSAGE *pTM);

GUI_TIMER_HANDLE GUI_TIMER_Create(GUI_TIMER_CALLBACK *cb, int Time, uint32_t Context, int Flags);
void             GUI_TIMER_Delete(GUI_TIMER_HANDLE hObj);

/* Methods changing properties */
void GUI_TIMER_SetPeriod(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Period);
void GUI_TIMER_SetTime(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Period);
void GUI_TIMER_SetDelay(GUI_TIMER_HANDLE hObj, GUI_TIMER_TIME Delay);
void GUI_TIMER_Restart(GUI_TIMER_HANDLE hObj);
int  GUI_TIMER_Exec(void);

void GUI_TIMER_Context(GUI_TIMER_HANDLE hObj, uintptr_t Context);	//houhh 20061020

/* Message layer */
void GUI_StoreKeyMsg(int Key, int Pressed);
void GUI_SendKeyMsg(int Key, int Pressed);
int  GUI_PollKeyMsg(void);

/* Application layer */
int  GUI_GetKey(void);
int  GUI_WaitKey(void);
void GUI_StoreKey(int c);
void GUI_ClearKeyBuffer(void);

void GUI_PID_StoreState(const GUI_PID_STATE *pState);
int  GUI_PID_GetState(GUI_PID_STATE *pState);

int  GUI_MOUSE_GetState(GUI_PID_STATE *pState);
void GUI_MOUSE_StoreState(const GUI_PID_STATE *pState);

int  GUI_TOUCH_GetState(GUI_PID_STATE *pState);
void GUI_TOUCH_GetUnstable(int *px, int *py);  /* for diagnostics only */
void GUI_TOUCH_StoreState(int x, int y);
void GUI_TOUCH_StoreStateEx(const GUI_PID_STATE *pState);
void GUI_TOUCH_StoreUnstable(int x, int y);

#define GUI_DispString_UC  GUI_UC_DispString
