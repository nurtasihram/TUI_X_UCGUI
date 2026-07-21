#include "WIDGET.h"

struct WIDGET_EFFECT_Simple : public WIDGET_EFFECT {
	void DrawUp(GUI_RECT r) const override {
		GUI_SetColor(RGB_BLACK);
		GUI_DrawRect(r);
	}
	void DrawDown(GUI_RECT r) const override {
		GUI_SetColor(RGB_BLACK);
		GUI_DrawRect(r);
	}
	WIDGET_EFFECT_Simple() : WIDGET_EFFECT(1) {}
};

struct WIDGET_EFFECT_3D : public WIDGET_EFFECT {
	void DrawUp(GUI_RECT r) const override {
		GUI_SetColor(RGB_BLACK);
		GUI_DrawRect(r); /* Draw rectangle around it */
		/* Draw the bright sides */
		GUI_SetColor(RGB_WHITE);
		GUI_DrawHLine(r.y0 + 1, r.x0 + 1, r.x1 - 2); /* Draw top line */
		GUI_DrawVLine(r.x0 + 1, r.y0 + 1, r.y1 - 2);
		/* Draw the dark sides */
		GUI_SetColor(RGB_GRAYL(0x55));
		GUI_DrawHLine(r.y1 - 1, r.x0 + 1, r.x1 - 1);
		GUI_DrawVLine(r.x1 - 1, r.y0 + 1, r.y1 - 2);
	}
	void DrawDown(GUI_RECT r) const override {
		GUI_SetColor(RGB_BLACK); /* TBD: Use halftone */
		/*  GUI_DrawRect(0, 0, r.x1, r.y1);*/
		/* Draw the upper left sides */
		GUI_SetColor(RGB_GRAYL(0x80));
		GUI_DrawHLine(r.y0, r.x0, r.x1);
		GUI_DrawVLine(r.x0, r.y0 + 1, r.y1);
		GUI_SetColor(0x0);
		GUI_DrawHLine(r.y0 + 1, r.x0 + 1, r.x1 - 1);
		GUI_DrawVLine(r.x0 + 1, r.y0 + 2, r.y1 - 1);
		/* Draw the lower right sides */
		GUI_SetColor(RGB_WHITE);
		GUI_DrawHLine(r.y1, r.x0 + 1, r.x1);
		GUI_DrawVLine(r.x1, r.y0 + 1, r.y1);
		GUI_SetColor(RGB_GRAYL(0xc0));
		GUI_DrawHLine(r.y1 - 1, r.x0 + 2, r.x1 - 1);
		GUI_DrawVLine(r.x1 - 1, r.y0 + 2, r.y1 - 1);
	}
	WIDGET_EFFECT_3D() : WIDGET_EFFECT(2) {}
};

struct WIDGET_EFFECT_3D1L : public WIDGET_EFFECT {
	void DrawUp(GUI_RECT r) const override {
		/* Draw the upper left sides */
		GUI_SetColor(RGB_GRAYL(0xE7));
		GUI_DrawHLine(r.y0, r.x0, r.x1 - 1);
		GUI_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
		/* Draw the lower right sides */
		GUI_SetColor(RGB_GRAYL(0x60));
		GUI_DrawHLine(r.y1, r.x0, r.x1);
		GUI_DrawVLine(r.x1, r.y0, r.y1 - 1);
	}
	void DrawDown(GUI_RECT r) const override {
		/* Draw the upper left sides */
		GUI_SetColor(RGB_GRAYL(0x60));
		GUI_DrawHLine(r.y0, r.x0, r.x1 - 1);
		GUI_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
		/* Draw the lower right sides */
		GUI_SetColor(RGB_GRAYL(0xE7));
		GUI_DrawHLine(r.y1, r.x0, r.x1);
		GUI_DrawVLine(r.x1, r.y0, r.y1 - 1);
	}
	WIDGET_EFFECT_3D1L() : WIDGET_EFFECT(1) {}
};

struct WIDGET_EFFECT_3D2L : public WIDGET_EFFECT {
	void DrawUp(GUI_RECT r) const override {
		/* Draw the upper left sides */
		GUI_SetColor(RGB_GRAYL(0xD0));
		GUI_DrawHLine(r.y0, r.x0, r.x1 - 1);
		GUI_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
		GUI_SetColor(RGB_GRAYL(0xE7));
		GUI_DrawHLine(r.y0 + 1, r.x0 + 1, r.x1 - 2);
		GUI_DrawVLine(r.x0 + 1, r.y0 + 2, r.y1 - 2);
		/* Draw the lower right sides */
		GUI_SetColor(RGB_GRAYL(0x60));
		GUI_DrawHLine(r.y1, r.x0, r.x1);
		GUI_DrawVLine(r.x1, r.y0, r.y1 - 1);
		GUI_SetColor(RGB_GRAYL(0x9A));
		GUI_DrawHLine(r.y1 - 1, r.x0 + 1, r.x1 - 1);
		GUI_DrawVLine(r.x1 - 1, r.y0 + 1, r.y1 - 2);
	}
	void DrawDown(GUI_RECT r) const override {
		/* Draw the upper left sides */
		GUI_SetColor(RGB_GRAYL(0x9A));
		GUI_DrawHLine(r.y0, r.x0, r.x1 - 1);
		GUI_DrawVLine(r.x0, r.y0 + 1, r.y1 - 1);
		GUI_SetColor(RGB_GRAYL(0x60));
		GUI_DrawHLine(r.y0 + 1, r.x0 + 1, r.x1 - 2);
		GUI_DrawVLine(r.x0 + 1, r.y0 + 2, r.y1 - 2);
		/* Draw the lower right sides */
		GUI_SetColor(RGB_GRAYL(0xE7));
		GUI_DrawHLine(r.y1, r.x0, r.x1);
		GUI_DrawVLine(r.x1, r.y0, r.y1 - 1);
		GUI_SetColor(RGB_GRAYL(0xD0));
		GUI_DrawHLine(r.y1 - 1, r.x0 + 1, r.x1 - 1);
		GUI_DrawVLine(r.x1 - 1, r.y0 + 1, r.y1 - 2);
	}
	WIDGET_EFFECT_3D2L() : WIDGET_EFFECT(2) {}
};


WIDGET_EFFECT _WIDGET_EFFECT_None{};
PCWIDGET_EFFECT WIDGET_Effect_None = &_WIDGET_EFFECT_None;

WIDGET_EFFECT_Simple _WIDGET_EFFECT_Simple{};
PCWIDGET_EFFECT WIDGET_Effect_Simple = &_WIDGET_EFFECT_Simple;

WIDGET_EFFECT_3D _WIDGET_EFFECT_3D{};
PCWIDGET_EFFECT WIDGET_Effect_3D = &_WIDGET_EFFECT_3D;

WIDGET_EFFECT_3D1L _WIDGET_EFFECT_3D1L{};
PCWIDGET_EFFECT WIDGET_Effect_3D1L = &_WIDGET_EFFECT_3D1L;

WIDGET_EFFECT_3D2L _WIDGET_EFFECT_3D2L{};
PCWIDGET_EFFECT WIDGET_Effect_3D2L = &_WIDGET_EFFECT_3D2L;

