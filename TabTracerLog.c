#include "TabTracerLog.h"

static HWND hLogWnd;
static HFONT hTimeLabelFont;
static HFONT hItemLabelFont;
static int cDebug_CBtype;
static COMBOBOXEXITEM cbexitem;
static int dbtypesel, dbelemsel;
/* Strings voor weergave in Debug window combo box */
/* Volgorde niet wijzigen ivw followTab */
static TCHAR typen[10][16] =
{ TEXT("Fase"),
	TEXT("Detector"),
	TEXT("Counter"),
	TEXT("Timer"),
	TEXT("Ingang"),
	TEXT("Uitgang"),
	TEXT("Hulpelement"),
	TEXT("Schakelaar"),
	TEXT("Parameter"),
	TEXT("Memoryelem") };
/* Variabelen voor scoll functies */
static int iVscrollPos;
static int iVscrollRange;
static int iVscrollMax;
static int iVscrollMin;
static int iHscrollPos;
static int iHscrollRange;
static int iHscrollMax;
static int iHscrollMin;

LRESULT CALLBACK WindowProcTabTracerLog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	HDC hdc;
	PAINTSTRUCT ps;
	HDC memHdc = NULL;
	POINT mousepos;
	HBITMAP bufBMP;

	switch (uMsg)
	{
	case WM_CREATE:

		GetClientRect(hWnd, &rect);
		
		iTracerTop = rect.top;
		iTracerLeft = rect.left;
		iTracerRight = rect.right;
		iTracerBottom = rect.bottom;

		/* Initiele default waarden (voor welke niet met settings worden geladen) */
		tracerlayout.rs_height = 3;
		tracerlayout.pad_left = 5;
		tracerlayout.pad_right = 50;
		tracerlayout.pad_bottom = 5;
		tracerlayout.text_top = 35;

		tracerlayout.fc_height = 22;
		tracerlayout.cg_height = 16;
		tracerlayout.rv_line_top = 10;
		tracerlayout.rv_line_bottom = 12;
		tracerlayout.pad_top = 22;
		tracerlayout.timestep = 16;
		tracerlayout.timeline = 5;
		tracerzoom = 1;

		hDebugCBtype = CreateWindowEx(
			0, WC_COMBOBOXEX, TEXT(""), CBS_DROPDOWN | WS_CHILD,
			iTracerLeft + 5, iTracerTop + 5, 150, 180,
			hWnd, (HMENU)DEBUGCOMBOTYPE, hMainInstance, NULL);
		hDebugCBelem = CreateWindowEx(
			0, WC_COMBOBOXEX, TEXT(""), CBS_DROPDOWN | WS_CHILD,
			iTracerLeft + 10 + 150, iTracerTop + 5, 150, 400,
			hWnd, (HMENU)DEBUGCOMBOELEM, hMainInstance, NULL);
		/* Aanmaken 'zoom switch' knop */
		hLogZoomswitch = CreateWindowEx(
			0, "BUTTON", "S/TS", WS_CHILD,
			iTracerLeft + 15 + 300, iTracerTop + 5, 40, 25,
			hWnd, (HMENU)BUTTONLOGZOOMSW, hMainInstance, NULL);
		/* Aanmaken 'zoom in' knop */
		hLogZoomin = CreateWindowEx(
			0, "BUTTON", "+", WS_CHILD,
			iTracerLeft + 20 + 340, iTracerTop + 5, 40, 25,
			hWnd, (HMENU)BUTTONLOGZOOMIN, hMainInstance, NULL);
		/* Aanmaken 'zoom out' knop */
		hLogZoomout = CreateWindowEx(
			0, "BUTTON", "-", WS_CHILD,
			iTracerLeft + 25 + 380, iTracerTop + 5, 40, 25,
			hWnd, (HMENU)BUTTONLOGZOOMUIT, hMainInstance, NULL);

		RegisterSomeClass(hMainInstance, "TabTracerLogLoggerPanel", (WNDPROC)wProc);
		int e = GetLastError();
		GetClientRect(hwndTab, &r);
		TabCtrl_AdjustRect(hwndTab, 0, &r);
		HWND hWnd = CreateWindow(
			pcClassName, "",
			WS_CHILD,
			r.left, r.top, r.right - r.left, r.bottom - r.top,
			hwndTab, NULL, hInstance, NULL);
		e = GetLastError();
		return hWnd;

		hTabs[TAB_HELPELEMS] = CreateTabDisplayWindow(hMainTab, hMainInstance, "MirakelTabHelpElemsClass", (WNDPROC)WindowProcTabHelpElements);

		InvalidateRect(hWnd, NULL, TRUE);

		// Set tab text font
		hTimeLabelFont = CreateFont(13, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, "Arial");
		hItemLabelFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Consolas");

		/* Vullen type combobox met mogelijk CCOL typen om te volgen */
		for (int i = 0; i <= 9; ++i)
		{
#ifdef MSVC6
			sprintf(lpszTemp1, typen[i]);
#else
			sprintf_s(lpszTemp1, SZBUFFERSIZE, typen[i]);
#endif
			cbexitem.mask = CBEIF_TEXT;
			cbexitem.pszText = lpszTemp1;
			cbexitem.iImage = 0;
			cbexitem.iItem = i;
			SendMessage(hDebugCBtype, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
		}
		SendMessage(hDebugCBtype, CB_SELECTSTRING, (WPARAM)0, (LPARAM)0);
		/* Vul alvast de tweede box met fasenummers */
		for (int i = 0; i < FC_MAX; ++i)
		{
#ifdef MSVC6
			sprintf(lpszTemp1, typen[i]);
#else
			sprintf_s(lpszTemp1, SZBUFFERSIZE, FC_code[i]);
#endif
			cbexitem.mask = CBEIF_TEXT;
			cbexitem.pszText = lpszTemp1;
			cbexitem.iImage = 0;
			cbexitem.iItem = i;
			SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
		}

		break;

	case WM_DESTROY:
		for (int i = 0; i < TRACES_MAX; ++i)
		{
			if (tracer[i])
				tabDebugTracer_deleteTrace(i);
		}
		break;

	case WM_LBUTTONDOWN:
		GetClientRect(hWnd, &rect);
		InvalidateRect(hWnd, &rect, TRUE);
		GetCursorPos(&mousepos);
		ScreenToClient(hWnd, &mousepos);
		//strTFBContent.iShowFasestat = (mousepos.y - 20) / 20;
		//if (mousepos.y < 20 || strTFBContent.iShowFasestat >= FC_MAX)strTFBContent.iShowFasestat = -1;
		break;

	case WM_PAINT:
	{
		if (tracerzoom == tracerzoom_SEC && !TS) break;

		GetClientRect(hWnd, &rect);
		
		hdc = GetDC(hWnd);
		memHdc = CreateCompatibleDC(hdc);
		bufBMP = CreateCompatibleBitmap(hdc, iTracerRight - iTracerLeft, iTracerBottom - iTracerTop);
		SelectObject(memHdc, bufBMP);

		int left = iTracerLeft + tracerlayout.pad_left;
		int top = tracerlayout.text_top;
		int right = iTracerRight - tracerlayout.pad_right;
		int bottom = iTracerBottom - tracerlayout.pad_bottom;

		SelectObject(memHdc, hPenMirakel[BRUSH_GRAY]);
		SelectObject(memHdc, hBrushMirakel[BRUSH_GRAY]);
		Rectangle(memHdc,
			0,
			top,
			iTracerRight,
			bottom);
		SelectObject(memHdc, hPenMirakel[BRUSH_LIGHTGRAY]);
		SelectObject(memHdc, hBrushMirakel[BRUSH_LIGHTGRAY]);
		Rectangle(memHdc,
			0,
			top,
			right,
			bottom);
		
		SelectObject(memHdc, GetStockObject(NULL_PEN));
		SelectObject(memHdc, hItemLabelFont);

		for (int tr = 0; tr < TRACES_MAX; ++tr)
		{
			if (tracer[tr])
			{
				tabDebugTracer_paint_trace(memHdc, tr, left, top + tracer[tr]->place * tracerlayout.fc_height, right, bottom);
			}
		}
		/* Bovenkant van de log schoonvegen voor schrijven time stamps */
		SelectObject(memHdc, hBrushMirakel[BRUSH_LIGHTGRAY]);
		SelectObject(memHdc, hPenMirakel[BRUSH_LIGHTGRAY]);
		Rectangle(memHdc, left + 1, top, right - 1, top + 22);
		/* Laden juist text layout */
		SelectObject(memHdc, hTimeLabelFont);
		SelectObject(memHdc, hPenMirakel[BRUSH_LIGHTGRAY]);
		if (tracerzoom == tracerzoom_TSEC)
		{
			for (int j = ((iHscrollMax - iHscrollPos) * tracerzoom) * tracerlayout.timestep + right - ((CIF_KLOK[_CIF_TSEC_TELLER]) % tracerlayout.timeline) * tracerlayout.timestep;
				j > left + tracerlayout.pad_left;
				j -= tracerlayout.timestep * tracerlayout.timeline)
			{
				MoveToEx(memHdc, j, top, NULL);
				LineTo(memHdc, j, bottom);
				trlogtempsec = ((((iHscrollMax - iHscrollPos) * tracerzoom) * tracerlayout.timestep + right - j) / tracerlayout.timestep);
				trlogtime = ((CIF_KLOK[_CIF_UUR] * 3600) + (CIF_KLOK[_CIF_MINUUT] * 60) + CIF_KLOK[_CIF_SECONDE]) - (trlogtempsec / 10);
				trlogtempsec = CIF_KLOK[_CIF_SECONDE];
#ifdef MSVC6
				sprintf(trlogtimestamp, "%2.2d:%2.2d:%2.2d", trlogtime / 3600, (trlogtime % 3600) / 60, (trlogtime % 3600) % 60);
#else
				sprintf_s(trlogtimestamp, SZBUFFERSIZE, "%2.2d:%2.2d:%2.2d", trlogtime / 3600, (trlogtime % 3600) / 60, (trlogtime % 3600) % 60);
#endif
				SetBkMode(memHdc, TRANSPARENT);
				TextOut(memHdc, j, top, trlogtimestamp, strlen(trlogtimestamp));
				SetBkMode(memHdc, OPAQUE);
			}
		}
		else if (tracerzoom == tracerzoom_SEC)
		{
			for (int j = ((((iHscrollMax - iHscrollPos) * tracerzoom) * tracerlayout.timestep) / 10) + right - tracerlayout.pad_right - (CIF_KLOK[_CIF_SECONDE] % tracerlayout.timeline) * tracerlayout.timestep;
				j > left + tracerlayout.pad_left;
				j -= tracerlayout.timestep * tracerlayout.timeline)
			{
				MoveToEx(memHdc, j, top + tracerlayout.pad_top, NULL);
				LineTo(memHdc, j, bottom);
				trlogtempsec = (((((iHscrollMax - iHscrollPos) * tracerzoom) * tracerlayout.timestep) / 10) + right - tracerlayout.pad_right - j) / tracerlayout.timestep;
				trlogtime = ((CIF_KLOK[_CIF_UUR] * 3600) + (CIF_KLOK[_CIF_MINUUT] * 60) + CIF_KLOK[_CIF_SECONDE]) - trlogtempsec;
				trlogtempsec = CIF_KLOK[_CIF_SECONDE];
#ifdef MSVC6
				sprintf(trlogtimestamp, "%2.2d:%2.2d:%2.2d", trlogtime / 3600, (trlogtime % 3600) / 60, (trlogtime % 3600) % 60);
#else
				sprintf_s(trlogtimestamp, SZBUFFERSIZE, "%2.2d:%2.2d:%2.2d", trlogtime / 3600, (trlogtime % 3600) / 60, (trlogtime % 3600) % 60);
#endif
				SetBkMode(memHdc, TRANSPARENT);
				TextOut(memHdc, j, top, trlogtimestamp, strlen(trlogtimestamp));
				SetBkMode(memHdc, OPAQUE);
			}
		}

		BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, iTracerRight - iTracerLeft, iTracerBottom - iTracerTop, memHdc, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		ReleaseDC(hWnd, hdc);
		return 1;
	}
	case WM_SIZE:
	{
		GetClientRect(hMainTab, &rect);
		TabCtrl_AdjustRect(hMainTab, 0, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, width, height, SWP_NOACTIVATE);
		GetClientRect(hWnd, &rect);
		InvalidateRect(hWnd, &rect, TRUE);

		iTracerTop = rect.top;
		iTracerLeft = rect.left;
		iTracerRight = rect.right;
		iTracerBottom = rect.bottom;

		/* Nieuwe verticale scroll info */
		SetScrollRange(hWnd, SB_VERT, 0, max(0, (tracer_count + 4 - height / tracerlayout.fc_height)), TRUE);
		if ((height / tracerlayout.fc_height) > tracer_count) iVscrollPos = 0;
		iVscrollPos = min(iVscrollPos, iVscrollMax);
		if (iVscrollPos != GetScrollPos(hWnd, SB_VERT))
		{
			SetScrollPos(hWnd, SB_VERT, iVscrollPos, TRUE);
		}
		if (iTracerRight - iTracerLeft > (TRACERBUFFERSIZE / tracerzoom) * tracerlayout.timestep)
			tracerlayout.timestep = (iTracerRight - iTracerLeft) / (TRACERBUFFERSIZE / tracerzoom);
		/* Nieuwe horizontale scroll info */
		int iHscrollPosTRA = iHscrollMax; // tijdelijke opslag van positie
		iHscrollMax = ((TRACERBUFFERSIZE / tracerzoom) - (int)((float)(iTracerRight - iTracerLeft) / (float)tracerlayout.timestep));
		SetScrollRange(hWnd, SB_HORZ, 0, iHscrollMax, TRUE);
		iHscrollPos = iHscrollMax - iHscrollPosTRA + iHscrollPos; // terugzetten positie
		if (iHscrollPos != GetScrollPos(hWnd, SB_HORZ))
		{
			SetScrollPos(hWnd, SB_HORZ, iHscrollPos, TRUE);
		}
		iHscrollPosTRA = iHscrollPos < 0 ? iHscrollMax : iHscrollPos;

		for (int tr = 0; tr < TRACES_MAX; ++tr)
		{
			if (tracer[tr])
			{
				SetItemWindowsPos(tr);
			}
		}

		break;
	}
	case MIRMSG_TABCHANGE:
		switch (wParam)
		{
		case SW_SHOW:
			ShowWindow(hDebugCBtype, SW_SHOW);
			ShowWindow(hDebugCBelem, SW_SHOW);
			ShowWindow(hLogZoomswitch, SW_SHOW);
			ShowWindow(hLogZoomin, SW_SHOW);
			ShowWindow(hLogZoomout, SW_SHOW);
			break;
		}
		return 0;
	case WM_NOTIFY:
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BUTTONLOGZOOMIN:
			tabDebugTracer_key(hWnd, (DWORD)VK_ADD);
			return 1;
			break;
		case BUTTONLOGZOOMUIT:
			tabDebugTracer_key(hWnd, (DWORD)VK_SUBTRACT);
			return 1;
			break;
		case BUTTONLOGZOOMSW:
			tabDebugTracer_key(hWnd, (DWORD)'Z');
			return 1;
			break;
		case DEBUGCOMBOTYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				int k = 0, j = 0, i = 0;
				cDebug_CBtype = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				ComboBox_ResetContent(hDebugCBelem);
				switch (cDebug_CBtype)
				{
				case Fase:
					for (k = 0; k < FC_MAX; ++k)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, FC_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, FC_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = k;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem,-1, FC_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Detector:
					for (k = 0; k < DP_MAX; ++k)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, D_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, D_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = k;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, D_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Schakelaar:
					for (k = 0; k < SCH_MAX; ++k)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, SCH_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, SCH_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = k;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, SCH_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Hulpelement:
					for (k = 0; k < HE_MAX; ++k)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, H_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, H_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = k;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, H_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Parameter:
					for (k = 0; k < PRM_MAX; ++k)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, PRM_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, PRM_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = k;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, PRM_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Timer:
					for (k = 0; k < TM_MAX; ++k)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, T_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, T_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = k;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, T_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Ingang:
					for (k = DP_MAX, j = 0; k < IS_MAX; ++k, ++j)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, IS_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, IS_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = j;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, IS_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Uitgang:
					for (k = FC_MAX, j = 0; k < US_MAX; ++k, ++j)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, US_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, US_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = j;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, US_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Counter:
					for (k = 0; k < CT_MAX; ++k)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, C_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, C_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = k;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, C_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				case Memoryelem:
					for (k = 0; k < ME_MAX; ++k)
					{
						cbexitem.mask = CBEIF_TEXT;
#ifdef MSVC6
						strcpy(lpszTemp1, MM_code[k]);
#else
						strcpy_s(lpszTemp1, SZBUFFERSIZE, MM_code[k]);
#endif
						cbexitem.pszText = lpszTemp1;
						cbexitem.iItem = k;
						SendMessage(hDebugCBelem, CBEM_INSERTITEM, (WPARAM)0, (LPARAM)&cbexitem);
						//ComboBox_InsertString(hDebugCBelem, -1, C_code[k]);
					}
					SendMessage(hDebugCBelem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					break;
				default:
					break;
				}
			}
			return 1;

			//case DEBUGCHOOSEFL:
		case DEBUGCOMBOELEM:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				// Toevoegen volg object
				if (tracer_count >= 20)
				{
					MessageBox(hWnd, "Maximaal 20 traces. Verwijder eerst een andere.", "Buffer vol...", MB_OK);
					return 0;
				}
				// 1. Binnenhalen selectie info
				if ((dbtypesel = ComboBox_GetCurSel(hDebugCBtype)) == CB_ERR)
				{
					MessageBox(hWnd, "Selecteer eerst een type.", "Geen selectie gevonden", MB_OK);
					return 0;
				}
				if ((dbelemsel = ComboBox_GetCurSel(hDebugCBelem)) == CB_ERR)
				{
					MessageBox(hWnd, "Selecteer eerst een element.", "Geen selectie gevonden", MB_OK);
					return 0;
				}
				if (dbtypesel == Schakelaar || dbtypesel == Parameter)
					MessageBox(hWnd, "Helaas, parameter en schakelaars kunnen met de tracer (nog) niet worden gevolgd.", "Selectie niet valide", MB_OK);
				if (dbtypesel == Ingang) dbelemsel += DP_MAX;
				if (dbtypesel == Uitgang) dbelemsel += FC_MAX;
				tabDebugTracer_newTrace(hWnd, dbtypesel, dbelemsel);
			}
			return 1;

		case BUTTONDELTRACE:
			for (int k = 0; k < TRACES_MAX; ++k)
				if (tracer[k] && (HWND)lParam == tracer[k]->delwin)
				{
					tabDebugTracer_deleteTrace(k);
				}
			InvalidateRect(hWnd, NULL, TRUE);
			return 1;
			break;
		// move item up
		case BUTTONTRACEUP:
		{
			int iItem = 0, iPlace = 0;
			// find clicked item
			for (int i = 0; i < TRACES_MAX; ++i)
			{
				if (tracer[i] && (HWND)lParam == tracer[i]->upwin)
				{
					iPlace = tracer[i]->place;
					iItem = i;
					break;
				}
			}
			if (iPlace <= 0) return 0;
			for (int i = 0; i < TRACES_MAX; ++i)
			{
				if (tracer[i] && tracer[i]->place == (iPlace - 1))
				{
					++tracer[i]->place;
					SetItemWindowsPos(i);
					break;
				}
			}
			--tracer[iItem]->place;
			SetItemWindowsPos(iItem);
			InvalidateRect(hWnd, NULL, TRUE);
			return 1;
		}
		break;
		case BUTTONTRACEDOWN:
		{
			int iPlace = 0, iItem = 0;
			for (int i = 0; i < TRACES_MAX; ++i)
			{
				// zoeken juiste trace bij geklikte knop
				if (tracer[i] && (HWND)lParam == tracer[i]->downwin)
				{
					// onthouden deze plek
					iPlace = tracer[i]->place;
					// onthouden welke trace in array
					iItem = i;
					break;
				}
			}
			for (int i = 0; i < TRACES_MAX; ++i)
			{
				if (tracer[i] && tracer[i]->place == (iPlace + 1))
				{
					--tracer[i]->place;
					SetItemWindowsPos(i);
					break;
				}
			}
			tracer[iItem]->place = tracer[iItem]->place + 1 > tracer_count - 1 ? tracer_count - 1 : tracer[iItem]->place + 1;
			SetItemWindowsPos(iItem);
			return 1;
		}
		break;
		default:
			break;
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void SetItemWindowsPos(int i)
{
	SetWindowPos(tracer[i]->delwin, HWND_TOP, iTracerRight - 25, iTracerTop + (tracer[i]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top, 20, 20, SWP_SHOWWINDOW);
	SetWindowPos(tracer[i]->upwin, HWND_TOP, iTracerRight - 45, iTracerTop + (tracer[i]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top, 20, 10, SWP_SHOWWINDOW);
	SetWindowPos(tracer[i]->downwin, HWND_TOP, iTracerRight - 45, iTracerTop + (tracer[i]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top + 10, 20, 10, SWP_SHOWWINDOW);
}

void tabDebugTracer_paint_trace(HDC hDC, INT trace, INT left, INT top, INT right, INT bottom)
{
	int tmp_zoomofset = (trindex - ((iHscrollMax - iHscrollPos) * tracerzoom)) % tracerzoom;
	int i, ii, j;
	/* Opslaan virtuele verticale scroll positie */
	int log_pad_top_tmp = top + tracerlayout.pad_top - iVscrollPos * tracerlayout.fc_height;
	/* resert tijdelijke variabelen */
	int cg_stat_tmp = -1, cg_pos_tmp = -1;
	int rs_stat_tmp = -1, rs_pos_tmp = -1;
	for (ii = tracerzoom, i = trindex - tracerzoom - ((iHscrollMax - iHscrollPos) * tracerzoom), // -  tmp_zoomofset,			// i: toegang tot juiste plek in buffer: index minus huidige positie tov max
		j = right - tracerlayout.pad_right,					// j: plek op het scherm: start is rechts gecorrigeerd voor de padding
		cg_pos_tmp = j,
		rs_pos_tmp = j;			// tijdelijke positie variabelen gelijk aan startpositie
		j > left + tracerlayout.pad_left && (ii + tracerzoom) < TRACERBUFFERSIZE; ii += tracerzoom)// voorwaarde: plek op scherm pas nog binnen het tekenvlak voor de log
	{
		/* Index binnen max houden */
		if (i < 0) i = ((i + TRACERBUFFERSIZE) % TRACERBUFFERSIZE);
		/* Indien de status van de trace bij start niet overeen komt met de status op deze plek in de log:
			- opslaan nieuwe status
			- opslaan startplek van nieuwe status */
		if (cg_stat_tmp != tracer[trace]->log[i][0])
		{
			cg_stat_tmp = tracer[trace]->log[i][0];
			cg_pos_tmp = j;
		}
		if (rs_stat_tmp != tracer[trace]->log[i][1])
		{
			rs_stat_tmp = tracer[trace]->log[i][1];
			rs_pos_tmp = j;
		}
		i = (((i - tracerzoom) + TRACERBUFFERSIZE) % TRACERBUFFERSIZE);
		/* Zolang het past binnen het tekenvlak, en er zijn geen wijzigingen in de status, teruglezen in de log */
		while (((j -= tracerlayout.timestep)) > (left + tracerlayout.pad_left) &&
			tracer[trace]->log[i][0] == cg_stat_tmp && tracer[trace]->log[i][1] == rs_stat_tmp
			&& (ii + tracerzoom) < TRACERBUFFERSIZE)
		{
			i = (((i - tracerzoom) + TRACERBUFFERSIZE) % TRACERBUFFERSIZE);
			ii += tracerzoom;
		}
		/* Indien er een wijziging is in de status, schilderen van de status tot nu toe.
			Indien het niet meer past, schilderen restant */
		if ((tracer[trace]->log[i][0] != cg_stat_tmp) || (j - tracerlayout.timestep < tracerlayout.pad_left + left) || (ii + tracerzoom) >= TRACERBUFFERSIZE)
		{
			if (j - tracerlayout.timestep < tracerlayout.pad_left + left)
				j = tracerlayout.pad_left + left;

			switch (tracer[trace]->type)
			{
			case Fase:
				SelectObject(hDC, hBrushCG[cg_stat_tmp]);
				SelectObject(hDC, hPenCG[cg_stat_tmp]);
				if (cg_stat_tmp == _CG_RV)
					Rectangle(hDC, j, log_pad_top_tmp + tracerlayout.rv_line_top, cg_pos_tmp, log_pad_top_tmp + tracerlayout.rv_line_bottom);
				//else if (cg_stat_tmp == CG_RV_OS)
				//	Rectangle(hDC, j, log_pad_top_tmp + tracerlayout.rv_line_top - 1, cg_pos_tmp, log_pad_top_tmp + tracerlayout.rv_line_bottom);
				else
					Rectangle(hDC, j, log_pad_top_tmp, cg_pos_tmp, log_pad_top_tmp + tracerlayout.cg_height);
				break;
			case Hulpelement:
			case Counter:
			case Timer:
			case Detector:
			case Ingang:
			case Uitgang:
			case Memoryelem:
				if (cg_stat_tmp)
				{
					SelectObject(hDC, hBrushCG[_CG_VS]);
					SelectObject(hDC, hPenCG[_CG_VS]);
					Rectangle(hDC, j, log_pad_top_tmp, cg_pos_tmp, log_pad_top_tmp + tracerlayout.cg_height);
				}
				else
				{
					SelectObject(hDC, hBrushMirakel[BRUSH_BLACK]);
					SelectObject(hDC, hPenMirakel[BRUSH_BLACK]);
					Rectangle(hDC, j, log_pad_top_tmp + tracerlayout.rv_line_top, cg_pos_tmp, log_pad_top_tmp + tracerlayout.rv_line_bottom);
				}
				break;
			default:
				break;
			}
		}
		/* Indien er een wijziging is in de realisatiestatus schilederen van de status tot nu toe.
			Indien het niet past, schilderen van restant */
		if ((tracer[trace]->log[i][1] != rs_stat_tmp) || (j - tracerlayout.timestep < tracerlayout.pad_left + left) || (ii + tracerzoom) >= TRACERBUFFERSIZE)
		{
			if (j - tracerlayout.timestep < tracerlayout.pad_left + left)
				j = tracerlayout.pad_left + left;
			SelectObject(hDC, hBrushMirakel[BRUSH_LIGHTGRAY]);
			SelectObject(hDC, hPenMirakel[BRUSH_LIGHTGRAY]);
			switch (tracer[trace]->type)
			{
			case Fase:
				if (rs_stat_tmp == 1)
				{
					SelectObject(hDC, hBrushMirakel[BRUSH_RED]); 
					SelectObject(hDC, hPenMirakel[BRUSH_LIGHTGRAY]);
				}
				if (rs_stat_tmp == 2)
				{
					SelectObject(hDC, hBrushMirakel[BRUSH_DARKGREEN]);
					SelectObject(hDC, hPenMirakel[BRUSH_DARKGREEN]);
				}
				if (rs_stat_tmp == 3)
				{
					SelectObject(hDC, hBrushMirakel[BRUSH_GREEN]);
					SelectObject(hDC, hPenMirakel[BRUSH_GREEN]);
				}
				if (rs_stat_tmp == 1)
					Rectangle(hDC, j, log_pad_top_tmp, j + 3, log_pad_top_tmp + tracerlayout.cg_height);
				else
					Rectangle(hDC, j, log_pad_top_tmp + tracerlayout.cg_height, rs_pos_tmp, log_pad_top_tmp + tracerlayout.rs_height + tracerlayout.cg_height);
				break;
			case Counter:
				if (rs_stat_tmp == 1) SelectObject(hDC, hBrushMirakel[BRUSH_RED]), SelectObject(hDC, hPenMirakel[BRUSH_RED]);
				if (rs_stat_tmp == 2) SelectObject(hDC, hBrushMirakel[BRUSH_GREEN]), SelectObject(hDC, hPenMirakel[BRUSH_GREEN]);
				Rectangle(hDC, j, log_pad_top_tmp + tracerlayout.cg_height, j + tracerlayout.timestep, log_pad_top_tmp + tracerlayout.rs_height + tracerlayout.cg_height);
				break;
			case Detector:
			case Hulpelement:
			case Uitgang:
			case Ingang:
			case Memoryelem:
				if (rs_stat_tmp == 1) SelectObject(hDC, hBrushMirakel[BRUSH_LIGHTBLUE]), SelectObject(hDC, hPenMirakel[BRUSH_LIGHTBLUE]);
				if (rs_stat_tmp == 2) SelectObject(hDC, hBrushMirakel[BRUSH_GREEN]), SelectObject(hDC, hPenMirakel[BRUSH_GREEN]);
				if (rs_stat_tmp == 3) SelectObject(hDC, hBrushMirakel[BRUSH_RED]), SelectObject(hDC, hPenMirakel[BRUSH_RED]);
				Rectangle(hDC, j, log_pad_top_tmp + tracerlayout.cg_height, rs_pos_tmp, log_pad_top_tmp + tracerlayout.rs_height + tracerlayout.cg_height);
				break;
			case Timer:
				if (rs_stat_tmp == 1) SelectObject(hDC, hBrushMirakel[BRUSH_LIGHTBLUE]), SelectObject(hDC, hPenMirakel[BRUSH_LIGHTBLUE]);
				if (rs_stat_tmp == 2) SelectObject(hDC, hBrushMirakel[BRUSH_BLUE]), SelectObject(hDC, hPenMirakel[BRUSH_BLUE]);
				if (rs_stat_tmp == 3) SelectObject(hDC, hBrushMirakel[BRUSH_DARKRED]), SelectObject(hDC, hPenMirakel[BRUSH_DARKRED]);
				if (rs_stat_tmp == 4) SelectObject(hDC, hBrushMirakel[BRUSH_RED]), SelectObject(hDC, hPenMirakel[BRUSH_RED]);
				Rectangle(hDC, j, log_pad_top_tmp + tracerlayout.cg_height, rs_pos_tmp, log_pad_top_tmp + tracerlayout.rs_height + tracerlayout.cg_height);
				break;
			default:
				break;
			}
		}
	}
	/* Weergeven labels */
	//TextOut(hDC, left + mir_i_char_w + 20, log_pad_top_tmp + tracerlayout.fc_height / 15, FC_code[_fc], min(6, strlen(FC_code[_fc])));
	int labelwidth = 50; 
	SetBkMode(hDC, TRANSPARENT);
	switch (tracer[trace]->type)
	{
	case Fase:
		TextOut(hDC, right - labelwidth, log_pad_top_tmp + tracerlayout.fc_height / 5 - 1, FC_code[tracer[trace]->elem], min(10, strlen(FC_code[tracer[trace]->elem])));
		break;
	case Detector:
		TextOut(hDC, right - labelwidth, log_pad_top_tmp + tracerlayout.fc_height / 5 - 1, D_code[tracer[trace]->elem], min(10, strlen(D_code[tracer[trace]->elem])));
		break;
	case Counter:
		TextOut(hDC, right - labelwidth, log_pad_top_tmp + tracerlayout.fc_height / 5 - 1, C_code[tracer[trace]->elem], min(10, strlen(C_code[tracer[trace]->elem])));
		break;
	case Timer:
		TextOut(hDC, right - labelwidth, log_pad_top_tmp + tracerlayout.fc_height / 5 - 1, T_code[tracer[trace]->elem], min(10, strlen(T_code[tracer[trace]->elem])));
		break;
	case Hulpelement:
		TextOut(hDC, right - labelwidth, log_pad_top_tmp + tracerlayout.fc_height / 5 - 1, H_code[tracer[trace]->elem], min(10, strlen(H_code[tracer[trace]->elem])));
		break;
	case Ingang:
		TextOut(hDC, right - labelwidth, log_pad_top_tmp + tracerlayout.fc_height / 5 - 1, IS_code[tracer[trace]->elem], min(10, strlen(IS_code[tracer[trace]->elem])));
		break;
	case Uitgang:
		TextOut(hDC, right - labelwidth, log_pad_top_tmp + tracerlayout.fc_height / 5 - 1, US_code[tracer[trace]->elem], min(10, strlen(US_code[tracer[trace]->elem])));
		break;
	case Memoryelem:
		TextOut(hDC, right - labelwidth, log_pad_top_tmp + tracerlayout.fc_height / 5 - 1, MM_code[tracer[trace]->elem], min(10, strlen(MM_code[tracer[trace]->elem])));
		break;
	default:
		break;
	}
	SetBkMode(hDC, OPAQUE);
}


void TabTracerLogUpdate()
{
	int n, p, q;
	int trindexprev = ((trindex - 1) + TRACERBUFFERSIZE) % TRACERBUFFERSIZE;

	if (TE)
	{
		tracer_bloklog[trindex] = ML;
		for (n = 0; n < TRACES_MAX; ++n)
		{
			if (tracer[n])
			{
				q = ((trindex + 1 + TRACERBUFFERSIZE) % TRACERBUFFERSIZE);
				for (p = trindex; p < q; ++p)
				{
					switch (tracer[n]->type)
					{
					case Fase:
						tracer[n]->log[p][FASESTATUS] = (char)CG[tracer[n]->elem];
						//if (tfbmonitor[tracer[n]->elem].overstaan && CG[tracer[n]->elem] == _CG_RV)
						//	tracer[n]->log[p][FASESTATUS] = (char)CG_RV_OS;
						if (AR[tracer[n]->elem]) tracer[n]->log[p][REALISATIESTATUS] = 3;
						else if (G[tracer[n]->elem]) tracer[n]->log[p][REALISATIESTATUS] = 2;
						else if (A[tracer[n]->elem]) tracer[n]->log[p][REALISATIESTATUS] = 1;
						else tracer[n]->log[p][REALISATIESTATUS] = 0;
						break;
					case Detector:
						tracer[n]->log[p][0] = (char)D[tracer[n]->elem];
						tracer[n]->log[p][1] = (char)TDH[tracer[n]->elem];
						break;
					case Hulpelement:
						tracer[n]->log[p][0] = (char)H[tracer[n]->elem];
						if (EH[tracer[n]->elem]) tracer[n]->log[p][1] = 3;
						else if (SH[tracer[n]->elem]) tracer[n]->log[p][1] = 2;
						else if (IH[tracer[n]->elem]) tracer[n]->log[p][1] = 1;
						else tracer[n]->log[p][1] = 0;
						break;
					case Ingang:
						tracer[n]->log[p][0] = (char)IS[tracer[n]->elem];
						break;
					case Uitgang:
						tracer[n]->log[p][0] = (char)US[tracer[n]->elem];
						tracer[n]->log[p][1] = (char)GUS[tracer[n]->elem];
						break;
					case Counter:
						tracer[n]->log[p][0] = (char)C_counter[tracer[n]->elem];
						tracer[n]->log[p][1] = C_counter[tracer[n]->elem] == 0 ? 0 : tracer[n]->log[p][1];
						tracer[n]->log[p][1] = C_counter[tracer[n]->elem] > tracer[n]->log[trindexprev][0] ? 2 : tracer[n]->log[p][1];
						tracer[n]->log[p][1] = C_counter[tracer[n]->elem] < tracer[n]->log[trindexprev][0] ? 1 : tracer[n]->log[p][1];
						break;
					case Timer:
						tracer[n]->log[p][0] = (char)T[tracer[n]->elem];
						tracer[n]->log[p][1] = 0;
						if (ET[tracer[n]->elem]) tracer[n]->log[p][1] = 4;
						if (HT[tracer[n]->elem]) tracer[n]->log[p][1] = 3;
						if (RT[tracer[n]->elem]) tracer[n]->log[p][1] = 2;
						if (IT[tracer[n]->elem]) tracer[n]->log[p][1] = 1;
						break;
					case Memoryelem:
						tracer[n]->log[p][0] = (char)MM[tracer[n]->elem];
						tracer[n]->log[p][1] = 0;
						break;
					default:
						break;
					}
				}
			}
		}
		trindex += 1;
		if (trindex >= TRACERBUFFERSIZE) trindex = 0;
	}
}

void TabTracerLogReset()
{
}

void tabDebugTracer_key(HWND hWnd, DWORD wKey)
{
	int tr;
	switch (wKey)
	{
		/* Faselog uitzoomen */
	case VK_SUBTRACT:
		if (GetKeyState(VK_SHIFT) & 0x80)
		{
			tracerlayout.fc_height = --tracerlayout.fc_height < 22 ? 22 : tracerlayout.fc_height;
			tracerlayout.cg_height = --tracerlayout.cg_height < 16 ? 16 : tracerlayout.cg_height;
			tracerlayout.rv_line_top = tracerlayout.fc_height / 2 - 1;
			tracerlayout.rv_line_bottom = tracerlayout.fc_height / 2 + 1;
			for (tr = 0; tr < TRACES_MAX; ++tr)
			{
				if (tracer[tr])
				{
					SetItemWindowsPos(tr);
				}
			}
		}
		else
		{
			tracerlayout.timestep = --tracerlayout.timestep < 1 ? 1 : tracerlayout.timestep;
			if (tracerlayout.timestep < 3)
				tracerlayout.timeline = 50;
			else if (tracerlayout.timestep < 4)
				tracerlayout.timeline = 20;
			else if (tracerlayout.timestep < 6)
				tracerlayout.timeline = 15;
			else if (tracerlayout.timestep < 10)
				tracerlayout.timeline = 10;
			else if (tracerzoom == tracerzoom_TSEC && tracerlayout.timestep == 50)
				tracerlayout.timeline = 1;
			else if (tracerzoom == tracerzoom_TSEC && tracerlayout.timestep == 25)
				tracerlayout.timeline = 2;
			else if (tracerzoom == tracerzoom_SEC && tracerlayout.timestep >= 10)
				tracerlayout.timeline = 5;
			if (iTracerRight - iTracerLeft > (TRACERBUFFERSIZE / tracerzoom) * tracerlayout.timestep)
				tracerlayout.timestep = (iTracerRight - iTracerLeft) / (TRACERBUFFERSIZE / tracerzoom);
		}
		/* Nieuwe horizontale scroll info */
		int iHscrollPosTRA = iHscrollMax; // tijdelijke opslag van positie
		iHscrollMax = ((TRACERBUFFERSIZE / tracerzoom) - (int)((float)(iTracerRight - iTracerLeft) / (float)tracerlayout.timestep));
		SetScrollRange(hWnd, SB_HORZ, 0, iHscrollMax, TRUE);
		iHscrollPos = iHscrollMax - iHscrollPosTRA + iHscrollPos; // terugzetten positie
		if (iHscrollPos != GetScrollPos(hWnd, SB_HORZ))
		{
			SetScrollPos(hWnd, SB_HORZ, iHscrollPos, TRUE);
		}
		iHscrollPosTRA = iHscrollPos < 0 ? iHscrollMax : iHscrollPos;
		/* Markeer de log als gewijzigd */
		InvalidateRect(hWnd, NULL, TRUE);
		tracerchanged = 1;
		break;

		/* Faselog uitzoomen */
	case VK_ADD:
		if (GetKeyState(VK_SHIFT) & 0x80)
		{
			tracerlayout.fc_height = ++tracerlayout.fc_height > 38 ? 38 : tracerlayout.fc_height;
			tracerlayout.cg_height = ++tracerlayout.cg_height > 32 ? 32 : tracerlayout.cg_height;
			tracerlayout.rv_line_top = tracerlayout.fc_height / 2 - 1;
			tracerlayout.rv_line_bottom = tracerlayout.fc_height / 2 + 1;
			for (tr = 0; tr < TRACES_MAX; ++tr)
			{
				if (tracer[tr])
				{
					SetItemWindowsPos(tr);
				}
			}
		}
		else
		{
			tracerlayout.timestep = ++tracerlayout.timestep > 50 ? 50 : tracerlayout.timestep;
			if (tracerlayout.timestep < 3)
				tracerlayout.timeline = 50;
			else if (tracerlayout.timestep < 4)
				tracerlayout.timeline = 20;
			else if (tracerlayout.timestep < 6)
				tracerlayout.timeline = 15;
			else if (tracerlayout.timestep < 10)
				tracerlayout.timeline = 10;
			else if (tracerzoom == tracerzoom_SEC && tracerlayout.timestep >= 10)
				tracerlayout.timeline = 5;
			else if (tracerzoom == tracerzoom_TSEC && tracerlayout.timestep == 50)
				tracerlayout.timeline = 1;
			else if (tracerzoom == tracerzoom_TSEC && tracerlayout.timestep >= 25)
				tracerlayout.timeline = 2;
		}
		/* Nieuwe horizontale scroll info */
		iHscrollPosTRA = iHscrollMax; // tijdelijke opslag van positie
		iHscrollMax = ((TRACERBUFFERSIZE / tracerzoom) - (int)((float)(iTracerRight - iTracerLeft) / (float)tracerlayout.timestep));
		SetScrollRange(hWnd, SB_HORZ, 0, iHscrollMax, TRUE);
		iHscrollPos = iHscrollMax - iHscrollPosTRA + iHscrollPos; // terugzetten positie
		if (iHscrollPos != GetScrollPos(hWnd, SB_HORZ))
		{
			SetScrollPos(hWnd, SB_HORZ, iHscrollPos, TRUE);
		}
		iHscrollPosTRA = iHscrollPos < 0 ? iHscrollMax : iHscrollPos;
		/* Markeer de log als gewijzigd */
		InvalidateRect(hWnd, NULL, TRUE);
		tracerchanged = 1;
		break;
	case 'Z':
		if (GetKeyState(VK_SHIFT) & 0x80)
			tracerzoom = tracerzoom = 5;
		else
			tracerzoom = tracerzoom == 1 ? 10 : 1;
		/* Nieuwe horizontale scroll info */
		iHscrollPosTRA = iHscrollMax; // tijdelijke opslag van positie
		iHscrollMax = (TRACERBUFFERSIZE / tracerzoom) - (int)((float)(iTracerRight - iTracerLeft) / (float)tracerlayout.timestep);
		SetScrollRange(hWnd, SB_HORZ, 0, iHscrollMax, TRUE);
		iHscrollPos = iHscrollMax - iHscrollPosTRA + iHscrollPos; // terugzetten positie
		if (iHscrollPos != GetScrollPos(hWnd, SB_HORZ))
		{
			SetScrollPos(hWnd, SB_HORZ, iHscrollPos, TRUE);
		}
		iHscrollPosTRA = iHscrollPos < 0 ? iHscrollMax : iHscrollPos;
		break;
	default:
		break;
	}
}


void tabDebugTracer_newTrace(HWND hWnd, int _type, int _elem)
{
	int tr;
	HINSTANCE hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	while (tracer[tracer_id])
		++tracer_id;
	tracer[tracer_id] = (TRACERSTRUCT *)malloc(sizeof(TRACERSTRUCT));

	tracer[tracer_id]->type = _type;
	tracer[tracer_id]->elem = _elem;
	tracer[tracer_id]->place = tracer_count;

	tracer[tracer_id]->delwin = CreateWindowEx(
		0, "BUTTON", "X", WS_CHILD | WS_VISIBLE | BS_FLAT,
		iTracerRight - 25, iTracerTop + (tracer[tracer_id]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top, 20, 20,
		hWnd, (HMENU)BUTTONDELTRACE, hInst, NULL);
	tracer[tracer_id]->upwin = CreateWindowEx(
		0, "BUTTON", "-", WS_CHILD | WS_VISIBLE | BS_FLAT,
		iTracerRight - 45, iTracerTop + (tracer[tracer_id]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top, 20, 10,
		hWnd, (HMENU)BUTTONTRACEUP, hInst, NULL);
	tracer[tracer_id]->downwin = CreateWindowEx(
		0, "BUTTON", "+", WS_CHILD | WS_VISIBLE | BS_FLAT,
		iTracerRight - 45, iTracerTop + (tracer[tracer_id]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top + 10, 20, 10,
		hWnd, (HMENU)BUTTONTRACEDOWN, hInst, NULL);

	for (tr = 0; tr < TRACERBUFFERSIZE; ++tr)
	{
		tracer[tracer_id]->log[tr][0] = _type == Fase ? 7 : 0;
		tracer[tracer_id]->log[tr][1] = 0;
	}

	tracer_id = ++tracer_id%TRACES_MAX;
	++tracer_count;
	tracerchanged = 1;
	InvalidateRect(hWnd, NULL, TRUE);
}

void tabDebugTracer_newTrace_create(HWND hWnd, int _type, int _elem, int _place)
{
	int tr;
	while (tracer[tracer_id])
		++tracer_id;
	tracer[tracer_id] = (TRACERSTRUCT *)malloc(sizeof(TRACERSTRUCT));

	tracer[tracer_id]->type = _type;
	tracer[tracer_id]->elem = _elem;
	tracer[tracer_id]->place = _place;

	tracer[tracer_id]->delwin = CreateWindowEx(
		0, "BUTTON", "X", WS_CHILD | WS_VISIBLE,
		iTracerRight - 25, iTracerTop + (tracer[tracer_id]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top, 20, 20,
		hWnd, (HMENU)BUTTONDELTRACE, hMainInstance, NULL);
	tracer[tracer_id]->upwin = CreateWindowEx(
		0, "BUTTON", "-", WS_CHILD | WS_VISIBLE,
		iTracerRight - 45, iTracerTop + (tracer[tracer_id]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top, 20, 10,
		hWnd, (HMENU)BUTTONTRACEUP, hMainInstance, NULL);
	tracer[tracer_id]->downwin = CreateWindowEx(
		0, "BUTTON", "+", WS_CHILD | WS_VISIBLE,
		iTracerRight - 45, iTracerTop + (tracer[tracer_id]->place + 1) * tracerlayout.fc_height + tracerlayout.text_top + 10, 20, 10,
		hWnd, (HMENU)BUTTONTRACEDOWN, hMainInstance, NULL);

	for (tr = 0; tr < TRACERBUFFERSIZE; ++tr)
	{
		tracer[tracer_id]->log[tr][0] = _type == Fase ? 7 : 0;
		tracer[tracer_id]->log[tr][1] = 0;
	}

	tracer_id = ++tracer_id%TRACES_MAX;
	++tracer_count;
	InvalidateRect(hWnd, NULL, TRUE);
}

void tabDebugTracer_deleteTrace(int id)
{
	int tr, prevpl;
	prevpl = tracer[id]->place;
	ShowWindow(tracer[id]->delwin, SW_HIDE);
	ShowWindow(tracer[id]->upwin, SW_HIDE);
	ShowWindow(tracer[id]->downwin, SW_HIDE);
	DestroyWindow(tracer[id]->delwin);
	DestroyWindow(tracer[id]->upwin);
	DestroyWindow(tracer[id]->downwin);
	free(tracer[id]);
	tracer[id] = NULL;
	--tracer_count;
	for (tr = 0; tr < TRACES_MAX; ++tr)
	{
		if (tracer[tr])
		{
			if (tracer[tr]->place > prevpl)
			{
				--tracer[tr]->place;
				SetWindowPos(tracer[tr]->delwin, HWND_TOP, iTracerRight - 30, iTracerTop + (tracer[tr]->place) * tracerlayout.fc_height + tracerlayout.pad_top, 20, 20, SWP_SHOWWINDOW);
				SetWindowPos(tracer[tr]->upwin, HWND_TOP, iTracerRight - 50, iTracerTop + (tracer[tr]->place) * tracerlayout.fc_height + tracerlayout.pad_top, 20, 10, SWP_SHOWWINDOW);
				SetWindowPos(tracer[tr]->downwin, HWND_TOP, iTracerRight - 50, iTracerTop + (tracer[tr]->place) * tracerlayout.fc_height + tracerlayout.pad_top + 10, 20, 10, SWP_SHOWWINDOW);
			}
		}
	}
	tracerchanged = 1;
}
