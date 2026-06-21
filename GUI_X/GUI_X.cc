#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#include "GUI.h"
#include "GUI_X.h"

volatile int OS_TimeMS;

int GUI_X_GetTime(void) {
	return (int)GetTickCount();
}

void GUI_X_Delay(int ms) {
	int tEnd = GUI_X_GetTime() + ms;
	while ((tEnd - GUI_X_GetTime()) > 0);
}

void GUI_X_Init(void) {}

void GUI_X_Log(const char *s, ...) {
	printf("LOG: ");
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
}

void GUI_X_Warn(const char *s, ...) {
	printf("WARN: ");
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
}

void GUI_X_ErrorOut(const char *s, ...) {
	printf("ERROR: ");
	va_list ap;
	va_start(ap, s);
	vprintf(s, ap);
	va_end(ap);
}
