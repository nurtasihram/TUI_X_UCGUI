#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

import TUX.X;

void GUI_X_Init(void) {}

int GUI_TIME_Get(void) {
	return (int)GetTickCount();
}

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
