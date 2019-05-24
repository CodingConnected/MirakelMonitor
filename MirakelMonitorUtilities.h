#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include "CCOLDefines.h"

typedef enum MirakelColorsEnum
{
	BRUSH_RED,
	BRUSH_DARKRED,
	BRUSH_GREEN,
	BRUSH_DARKGREEN,
	BRUSH_BLUE,
	BRUSH_DARKBLUE,
	BRUSH_YELLOW,
	BRUSH_ORANGE,
	BRUSH_MAGENTA,
	BRUSH_TEAL,
	BRUSH_WHITE,
	BRUSH_BLACK,
	BRUSH_GRAY,
	BRUSH_LIGHTGRAY,
	BRUSH_DARKGRAY,
	BRUSH_MAX
};

extern HBRUSH hBrushMirakel[BRUSH_MAX];
extern HBRUSH hBrushCG[_CG_MAX + 1];
extern HPEN hPenMirakel[BRUSH_MAX];
extern HWND hMainTab;
extern HINSTANCE hMainInstance;

extern int iCharWidth, iCharHeight;

extern COLORREF greenprtext;
extern COLORREF greenartext;
extern COLORREF redtext;
extern COLORREF whitetext;
extern COLORREF blacktext;

#define SZBUFFERSIZE 128

extern char lpszTemp1[SZBUFFERSIZE];

void Initialize(HWND hWnd);
void CreateBrushes();
HWND CreateTabDisplayWindow(HWND hwndTab, HINSTANCE hInstance, char * pcClassName, WNDPROC wProc);
ATOM RegisterSomeClass(HINSTANCE hInstance, char * className, WNDPROC proc);
