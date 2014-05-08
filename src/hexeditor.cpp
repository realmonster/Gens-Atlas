#include "resource.h"
#include "gens.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "mem_z80.h"
#include "G_main.h"
#include "G_ddraw.h"
#include "G_dsound.h"
#include "ram_search.h"
#include "hexeditor.h"
#include "save.h"
#include <windowsx.h>

HWND HexEditorHWnd;
HMENU HexEditorMenu;
HDC HexDC;
SCROLLINFO HexSI;
MousePos MouseArea = NO;

bool
	MouseButtonHeld = 0,
	DrawLines = 1,
	HexStarted = 0;

unsigned int
	ClientTopGap = 0,	// How much client area is shifted
	ClientXGap = 0,		// Total diff between client and dialog widths
	RowCount = 16;		// Offset consists of 16 bytes
/*
HexRegion HexRegions[] = {
	{"ROM",			0,								0,			0,					16},
	{"RAM CD PRG",	(unsigned char *)Ram_Prg,		0x020000,	SEGACD_RAM_PRG_SIZE,16},
	{"RAM CD 1M",	(unsigned char *)Ram_Word_1M,	0x200000,	SEGACD_1M_RAM_SIZE,	16},
	{"RAM CD 2M",	(unsigned char *)Ram_Word_2M,	0x200000,	SEGACD_2M_RAM_SIZE,	16},
	{"RAM Z80",		(unsigned char *)Ram_Z80,		0xA00000,	Z80_RAM_SIZE,		16},
	{"RAM M68K",	(unsigned char *)Ram_68k,		0xFF0000,	_68K_RAM_SIZE,		16},
	{"RAM 32X",		(unsigned char *)_32X_Ram,		0x06000000,	_32X_RAM_SIZE,		16},
};
*/
HexParameters Hex = {
	1,												// text area visible
	0, 15, Hex.FontHeight/2, Hex.FontBold? 600:400,	// font				// bold, height, width, weight
	8, 0,											// font gap			// left, top
	Hex.FontWidth * 6,								// header gap		// X
	Hex.FontHeight + Hex.GapFontY,					// header gap		// Y
	Hex.FontHeight  + Hex.GapFontY,					// cell				// height
	Hex.FontWidth*2 + Hex.GapFontX,					// cell				// width
	0, 0,											// dialog pos		// X, Y
	0, 16,											// visible offset	// first, total
	0, 0, 0,										// selected address // first, total, last
	0xff0000,										// memory region	// m68k ram
	0x00000000, 0x00ffffff							// colors			// font, BG
};

RECT
	CellArea = {
		Hex.GapHeaderX + Hex.FontWidth * 2,
		Hex.GapHeaderY,
		Hex.GapHeaderX + Hex.FontWidth * 2 + Hex.CellWidth * RowCount,
		Hex.GapHeaderY + Hex.CellHeight* Hex.OffsetVisibleTotal
	},
	TextArea = {
		Hex.GapHeaderX + Hex.FontWidth * 2 + Hex.CellWidth * RowCount,
		Hex.GapHeaderY,
		Hex.GapHeaderX + Hex.FontWidth * 3 + Hex.CellWidth * RowCount + Hex.FontWidth * RowCount,
		Hex.GapHeaderY + Hex.CellHeight* Hex.OffsetVisibleTotal
	};

#define CLIENT_WIDTH	(Hex.TextView ? TextArea.right : TextArea.left)
#define CLIENT_HEIGHT	(Hex.CellHeight * (Hex.OffsetVisibleTotal + 1) + 1)
#define LAST_OFFSET		(Hex.OffsetVisibleFirst + Hex.OffsetVisibleTotal)
#define LAST_ADDRESS	(Hex.OffsetVisibleFirst + Hex.OffsetVisibleTotal * RowCount - 1)
#define SELECTION_START	min(Hex.AddressSelectedFirst, Hex.AddressSelectedLast)
#define SELECTION_END	max(Hex.AddressSelectedFirst, Hex.AddressSelectedLast)

HFONT HexFont = 0;

void HexSetColors(HDC hDC, bool Selection) {
	if (Selection) {
		SetBkColor(hDC, Hex.ColorFont);
		SetTextColor(hDC, Hex.ColorBG);		
	} else {
		SetBkColor(hDC, Hex.ColorBG);
		SetTextColor(hDC, Hex.ColorFont);
	}
}

void HexUpdateDialog(bool ClearBG) {
	if (ClearBG)
		InvalidateRect(HexEditorHWnd, NULL, TRUE);
	else
		InvalidateRect(HexEditorHWnd, NULL, FALSE);
}

void HexUpdateCaption() {
	char str[100];
	char area[10];
	if (MouseArea == TEXT)
		sprintf(area, "CHARS");
	else
		sprintf(area, "RAM M68K");
	if (Hex.AddressSelectedTotal == 0)
		sprintf(str, "RAM Dump: %s", area);
	else if (Hex.AddressSelectedTotal == 1)
		sprintf(str, "%s: $%06X", area,
				Hex.AddressSelectedFirst + Hex.MemoryRegion);
	else if (Hex.AddressSelectedTotal > 1)
		sprintf(str, "%s: $%06X - $%06X (%d)", area,
				SELECTION_START + Hex.MemoryRegion,
				SELECTION_END + Hex.MemoryRegion,
				Hex.AddressSelectedTotal);
	SetWindowText(HexEditorHWnd, str);
	return;
}

void HexUpdateScrollInfo() {
	ZeroMemory(&HexSI, sizeof(SCROLLINFO));
	HexSI.cbSize = sizeof(HexSI);
	HexSI.fMask  = SIF_ALL;
	HexSI.nMin   = 0;
	HexSI.nMax   = _68K_RAM_SIZE / RowCount;
	HexSI.nPage  = Hex.OffsetVisibleTotal;
	HexSI.nPos   = Hex.OffsetVisibleFirst / RowCount;
}

int HexGetMouseAddress(LPARAM lParam) {
	int Address;
	POINT Mouse;
	POINTSTOPOINT(Mouse, MAKEPOINTS(lParam));
	if (Mouse.x <  CellArea.left ) Mouse.x = CellArea.left;
	if (Mouse.x >= TextArea.right) Mouse.x = TextArea.right - 1;
	if (Mouse.x < CellArea.right) {
		MouseArea = CELL;
		Address = (Mouse.y - CellArea.top ) / (int)Hex.CellHeight * RowCount +
				  (Mouse.x - CellArea.left) / (int)Hex.CellWidth + Hex.OffsetVisibleFirst;
	} else if (Mouse.x >= CellArea.right && Hex.TextView) {
		MouseArea = TEXT;
		Address = (Mouse.y - TextArea.top ) / (int)Hex.CellHeight * RowCount +
				  (Mouse.x - TextArea.left) / (int)Hex.FontWidth + Hex.OffsetVisibleFirst;
	} else {
		MouseArea = NO;
		Address = -1;
	}
	return Address;
}

void HexSelectAddress(int Address, bool ButtonDown) {
	if (MouseArea == NO) return;
	else {
		if (Address < 0                ) Address = 0;
		if (Address > _68K_RAM_SIZE - 1) Address = _68K_RAM_SIZE - 1;
		if (ButtonDown) {
			Hex.AddressSelectedFirst = Address;
			Hex.AddressSelectedLast  = Address;
			Hex.AddressSelectedTotal = 1;
		} else {
			Hex.AddressSelectedLast  = Address;
			Hex.AddressSelectedTotal = SELECTION_END - SELECTION_START + 1;
		}
		if (Hex.AddressSelectedLast < Hex.OffsetVisibleFirst)
			Hex.OffsetVisibleFirst = SELECTION_START / RowCount * RowCount;
		if (Hex.AddressSelectedLast > LAST_ADDRESS)
			Hex.OffsetVisibleFirst = (SELECTION_END / RowCount - Hex.OffsetVisibleTotal + 1) * RowCount;
		HexUpdateDialog();
	}
}

void HexGoToAddress(int Address) {
	if (Address < 0                ) Address = 0;
	if (Address > _68K_RAM_SIZE - 1) Address = _68K_RAM_SIZE - 1;
	Hex.AddressSelectedFirst = Address;
	Hex.AddressSelectedLast  = Address;
	Hex.AddressSelectedTotal = 1;
	if (Hex.AddressSelectedLast < Hex.OffsetVisibleFirst)
		Hex.OffsetVisibleFirst = SELECTION_START / RowCount * RowCount;
	if (Hex.AddressSelectedLast > LAST_ADDRESS)
		Hex.OffsetVisibleFirst = (SELECTION_END / RowCount - Hex.OffsetVisibleTotal + 1) * RowCount;
	HexUpdateDialog();
}

void HexCopy(bool Text)
{
	if (!OpenClipboard(NULL)) return;
	if (!EmptyClipboard()) return;

	char str[10];
	HGLOBAL hGlobal = GlobalAlloc(GHND,Hex.AddressSelectedTotal*2+1);
	PTSTR pGlobal = (char*)GlobalLock (hGlobal);
	if (Text)
	{
		for(UINT i = 0; i < Hex.AddressSelectedTotal; i++)
		{
			UINT8 check = Ram_68k[(i+SELECTION_START)^1];
			//if((check >= 32) && (check <= 127))
				pGlobal[i] = (char) check;
			//else
			//	pGlobal[i] = '.';
		}
		pGlobal[Hex.AddressSelectedTotal] = 0;
	}
	else
	{
		for(UINT i = 0; i < Hex.AddressSelectedTotal; i++)
		{
			sprintf(str,"%02X",Ram_68k[(i+SELECTION_START)^1]);
			strcat(pGlobal,str);
		}
	}
	GlobalUnlock(hGlobal);
	SetClipboardData(CF_TEXT, hGlobal);
	CloseClipboard();
	GlobalFree(hGlobal);
}

void HexDestroySelection() {
	MouseArea = NO;
	Hex.AddressSelectedFirst = 0;
	Hex.AddressSelectedTotal = 0;
	Hex.AddressSelectedLast = 0;
}

void HexDestroyDialog() {
	RECT r;
	GetWindowRect(HexEditorHWnd, &r);
	Hex.DialogPosX = r.left;
	Hex.DialogPosY = r.top;
	DialogsOpen--;
	HexDestroySelection();
	ReleaseDC(HexEditorHWnd, HexDC);
	DestroyWindow(HexEditorHWnd);
	UnregisterClass("HEXEDITOR", ghInstance);
	DeleteObject(HexFont);
	HexFont = 0;
	HexEditorHWnd = 0;
	HexStarted = 0;
	return;
}

LRESULT CALLBACK HexGoToProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	RECT hr;
	switch(uMsg) {
		case WM_INITDIALOG:
			Clear_Sound_Buffer();
			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}
			GetWindowRect(HexEditorHWnd, &hr);
			SetWindowPos(hDlg, NULL, hr.left, hr.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			strcpy(Str_Tmp, "Type address to go to.");
			SendDlgItemMessage(hDlg, IDC_PROMPT_TEXT, WM_SETTEXT, 0, (LPARAM)Str_Tmp);
			strcpy(Str_Tmp, "Format: FF****");
			SendDlgItemMessage(hDlg, IDC_PROMPT_TEXT2, WM_SETTEXT, 0, (LPARAM)Str_Tmp);
			return true;
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
			case IDOK: {
				if (Full_Screen) {
					while (ShowCursor(true) < 0);
					while (ShowCursor(false) >= 0);
				}
				GetDlgItemText(hDlg, IDC_PROMPT_EDIT, Str_Tmp, 10);
				int Address;
				if ((strnicmp(Str_Tmp, "ff", 2) == 0) && (sscanf(Str_Tmp+2, "%x", &Address)))
					HexGoToAddress(Address);
				DialogsOpen--;
				EndDialog(hDlg, true);
				return true;
				}
				break;
			case ID_CANCEL:
			case IDCANCEL:
				if (Full_Screen) {
					while (ShowCursor(true) < 0);
					while (ShowCursor(false) >= 0);
				}
				DialogsOpen--;
				EndDialog(hDlg, false);
				return false;
				break;
			}
			break;

		case WM_CLOSE:
			if (Full_Screen) {
				while (ShowCursor(true) < 0);
				while (ShowCursor(false) >= 0);
			}
			DialogsOpen--;
			EndDialog(hDlg, false);
			return false;
			break;
	}
	return false;
}

LRESULT CALLBACK HexEditorProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r, wr, cr;
	PAINTSTRUCT ps;
	switch (uMsg) {
	case WM_CREATE: {
		HexDC = GetDC(hDlg);
		HexEditorMenu = GetMenu(hDlg);
		SelectObject(HexDC, HexFont);
		SetTextAlign(HexDC, TA_UPDATECP | TA_TOP | TA_LEFT);
		if (Full_Screen) {
			while (ShowCursor(false) >= 0);
			while (ShowCursor(true) < 0);
		}
		SetRect(&r, 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT);
		// Automatic adjust to account for menu and OS style, manual for scrollbar
		int ScrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
		AdjustWindowRectEx(&r, GetWindowLong(hDlg, GWL_STYLE),
			(GetMenu(hDlg) > 0), GetWindowLong(hDlg, GWL_EXSTYLE));
		SetWindowPos(hDlg, NULL, Hex.DialogPosX, Hex.DialogPosY,
			r.right - r.left + ScrollbarWidth, r.bottom - r.top,
			SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
		GetClientRect(hDlg, &cr);
		ClientTopGap = r.bottom - r.top - cr.bottom + 1;
		ClientXGap = r.right - r.left - CLIENT_WIDTH + ScrollbarWidth;
		Hex.AddressSelectedTotal = 0;
		HexUpdateScrollInfo();
		SetScrollInfo(hDlg, SB_VERT, &HexSI, TRUE);
		HexStarted = 1;
		return 0;
		}
		break;

	case WM_PAINT: {
		static char buf[10];
		unsigned int row = 0, line = 0;
		GetWindowRect(hDlg, &wr);
		BeginPaint(hDlg, &ps);
		// TOP HEADER, static.
		for (row = 0; row < RowCount; row++) {
			MoveToEx(HexDC, row * Hex.CellWidth + CellArea.left, -1, NULL);
			HexSetColors(HexDC, 0);
			sprintf(buf, "%2X", row);
			TextOut(HexDC, 0, 0, buf, strlen(buf));
		}
		// LEFT HEADER, semi-dynamic.
		for (line = 0; line < Hex.OffsetVisibleTotal; line++) {
			MoveToEx(HexDC, 0, line * Hex.CellHeight + CellArea.top, NULL);
			HexSetColors(HexDC, 0);
			sprintf(buf, "%06X:", Hex.OffsetVisibleFirst + line * RowCount + Hex.MemoryRegion);
			TextOut(HexDC, 0, 0, buf, strlen(buf));
		}
		// RAM, dynamic.
		for (line = 0; line < Hex.OffsetVisibleTotal; line++) {
			for (row = 0; row < RowCount; row++) {
				unsigned int carriage = Hex.OffsetVisibleFirst + line * RowCount + row;
				
				// Print numbers in main area
				MoveToEx(HexDC, row * Hex.CellWidth + CellArea.left,
					line * Hex.CellHeight + CellArea.top, NULL);
				if ((Hex.AddressSelectedTotal)
					&& (carriage >= SELECTION_START)
					&& (carriage <= SELECTION_END))
					HexSetColors(HexDC, 1);
				else
					HexSetColors(HexDC, 0);
				sprintf(buf, "%02X", Ram_68k[carriage^1]);
				TextOut(HexDC, 0, 0, buf, strlen(buf));
				// Print chars on the right
				if (Hex.TextView) {
					MoveToEx(HexDC, row * Hex.FontWidth + TextArea.left,
						line * Hex.CellHeight + CellArea.top, NULL);
					if ((Hex.AddressSelectedTotal) && (carriage >= SELECTION_START) && (carriage <= SELECTION_END))
						HexSetColors(HexDC, 1);
					else
						HexSetColors(HexDC, 0);
					UINT8 check = Ram_68k[carriage^1];
					if((check >= 32) && (check <= 127))
						buf[0] = (char) check;
					else
						buf[0] = '.';
					TextOut(HexDC, 0, 0, buf, 1);
				}
			}
		}
		// Some lines
		if (DrawLines) {
			MoveToEx(HexDC, 0, CellArea.top - 1, NULL);
			LineTo(HexDC, CLIENT_WIDTH, CellArea.top - 1); // horizontal
			MoveToEx(HexDC, CellArea.left - Hex.FontWidth / 2 - 1, 0, NULL);
			LineTo(HexDC, CellArea.left - Hex.FontWidth / 2 - 1, CLIENT_HEIGHT); // vertical left
			MoveToEx(HexDC, CellArea.left + Hex.CellWidth * 8 - Hex.FontWidth / 2 - 1, 0, NULL);
			LineTo(HexDC, CellArea.left + Hex.CellWidth * 8 - Hex.FontWidth / 2 - 1, CLIENT_HEIGHT); // vertical middle
			if (Hex.TextView) {
				MoveToEx(HexDC, TextArea.left - Hex.FontWidth / 2 - 1, 0, NULL);
				LineTo(HexDC, TextArea.left - Hex.FontWidth / 2 - 1, CLIENT_HEIGHT); // vertical right
			}
		}
		EndPaint(hDlg, &ps);
		return 0;
		}
		break;

	case WM_INITMENU:
		CheckMenuItem(HexEditorMenu, IDC_C_HEX_LINES, DrawLines ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(HexEditorMenu, IDC_C_HEX_TEXT, Hex.TextView ? MF_CHECKED : MF_UNCHECKED);
		break;

	case WM_MENUSELECT:
 	case WM_ENTERSIZEMOVE:
		Clear_Sound_Buffer();
		break;

	case WM_COMMAND: {
		switch(wParam) {

		case IDC_C_HEX_LINES:
			DrawLines = !DrawLines;
			CheckMenuItem(HexEditorMenu, IDC_C_HEX_LINES, DrawLines ? MF_CHECKED : MF_UNCHECKED);
			HexUpdateDialog(1);
			break;

		case IDC_C_HEX_TEXT:
			Hex.TextView = !Hex.TextView;
			CheckMenuItem(HexEditorMenu, IDC_C_HEX_TEXT, Hex.TextView ? MF_CHECKED : MF_UNCHECKED);
			GetWindowRect(hDlg, &wr);
			SetWindowPos(hDlg, NULL, wr.left, wr.top, CLIENT_WIDTH + ClientXGap, wr.bottom - wr.top,
				SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
			HexUpdateDialog();
			break;

		case IDC_C_HEX_GOTO:
			DialogsOpen++;
			DialogBox(ghInstance, MAKEINTRESOURCE(IDD_PROMPT), hDlg, (DLGPROC) HexGoToProc);
			break;

		case IDC_C_HEX_DUMP: {
				char fname[2048];
				strcpy(fname,"dump.bin");
				if(Change_File_S(fname,".","Save Full Dump As...","All Files\0*.*\0\0","*.*",hDlg)) {
					FILE *out=fopen(fname, "wb+");
					int i;
					for (i=0; i < sizeof(Ram_68k); ++i) {
						fname[i&2047] = Ram_68k[i^1];
						if ((i&2047) == 2047)
							fwrite(fname, 1, sizeof(fname), out);
					}
					fwrite(fname, 1, i&2047, out);
					fclose(out);
				}
			}
			break;
		case IDC_C_HEX_COPY_NUMS:
			HexCopy(0);
			break;
		case IDC_C_HEX_COPY_CHARS:
			HexCopy(1);
			break;
		case IDC_C_HEX_COPY_AUTO:
			HexCopy(MouseArea == TEXT);
			break;
		}
		}
		break;

	case WM_KEYDOWN:
		if (GetKeyState(VK_CONTROL) & 0x8000) {
			switch(wParam) {
			case 0x43: // Ctrl+C
				HexEditorProc(HexEditorHWnd, WM_COMMAND, IDC_C_HEX_COPY_AUTO, 0);
				return 0;
			case 0x47: // Ctrl+G
				HexEditorProc(HexEditorHWnd, WM_COMMAND, IDC_C_HEX_GOTO, 0);
				return 0;
			default:
				return 0;
			}
		} else
			return 0;
	
	case WM_LBUTTONDOWN:
		SetCapture(hDlg); // Watch mouse actions outside the client area
		HexSelectAddress(HexGetMouseAddress(lParam), 1);
		MouseButtonHeld = 1;
		HexUpdateCaption();
		return 0;
		break;

	case WM_MOUSEMOVE:
		HexGetMouseAddress(lParam); // Update mouse area
		if (MouseButtonHeld)
			HexSelectAddress(HexGetMouseAddress(lParam), 0);
		HexUpdateScrollInfo();
		SetScrollInfo(hDlg, SB_VERT, &HexSI, TRUE);
		HexUpdateCaption();
		return 0;
		break;

	case WM_LBUTTONUP:
		HexSelectAddress(HexGetMouseAddress(lParam), 0);
		MouseButtonHeld = 0;
		HexUpdateCaption();
		ReleaseCapture(); // Stop wathcing mouse
		return 0;
		break;

	case WM_VSCROLL:
		Clear_Sound_Buffer();
		HexUpdateScrollInfo();
		GetScrollInfo(hDlg, SB_VERT, &HexSI);
		switch (LOWORD(wParam)) {
		case SB_ENDSCROLL:
		case SB_TOP:
		case SB_BOTTOM:
			break;
		case SB_LINEUP:
			HexSI.nPos--;
			break;
		case SB_LINEDOWN:
			HexSI.nPos++;
			break;
		case SB_PAGEUP:
			HexSI.nPos -= HexSI.nPage;
			break;
		case SB_PAGEDOWN:
			HexSI.nPos += HexSI.nPage;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			HexSI.nPos = HexSI.nTrackPos;
			break;
		}
		if (HexSI.nPos < HexSI.nMin) HexSI.nPos = HexSI.nMin;
		if ((HexSI.nPos + (int)HexSI.nPage) > HexSI.nMax) HexSI.nPos = HexSI.nMax - HexSI.nPage;			
		Hex.OffsetVisibleFirst = HexSI.nPos * RowCount;
		SetScrollInfo(hDlg, SB_VERT, &HexSI, TRUE);
		HexUpdateDialog();
		return 0;
		break;

	case WM_MOUSEWHEEL: {
		int WheelDelta = (short)HIWORD(wParam);
		HexUpdateScrollInfo();
		GetScrollInfo(hDlg, SB_VERT, &HexSI);
		if (WheelDelta < 0) HexSI.nPos += HexSI.nPage;
		if (WheelDelta > 0) HexSI.nPos -= HexSI.nPage;
		if (HexSI.nPos < HexSI.nMin) HexSI.nPos = HexSI.nMin;
		if ((HexSI.nPos + (int)HexSI.nPage) > HexSI.nMax) HexSI.nPos = HexSI.nMax - HexSI.nPage;
		Hex.OffsetVisibleFirst = HexSI.nPos * RowCount;
		SetScrollInfo(hDlg, SB_VERT, &HexSI, TRUE);
		HexUpdateDialog();
		return 0;
		}
		break;

	case WM_SIZING: {
		Clear_Sound_Buffer();
		RECT *r = (RECT *) lParam;			
		HexUpdateScrollInfo();
		GetScrollInfo(hDlg, SB_VERT, &HexSI);
		if ((wParam == WMSZ_BOTTOM) || (wParam == WMSZ_BOTTOMRIGHT) || (wParam == WMSZ_RIGHT)) {
			// Gradual resizing
			unsigned int height = r->bottom - r->top;
			unsigned int width = r->right - r->left;
			// Manual adjust to account for cell parameters
			r->bottom = r->top + height - ((height - ClientTopGap) % Hex.CellHeight);
			HexSI.nPage = (height - ClientTopGap) / Hex.CellHeight - 1;
			if ((HexSI.nPos + (int) HexSI.nPage) > HexSI.nMax)
				HexSI.nPos = HexSI.nMax - HexSI.nPage;
			Hex.OffsetVisibleFirst = HexSI.nPos * RowCount;
			Hex.OffsetVisibleTotal = HexSI.nPage;
			SetScrollInfo(hDlg, SB_VERT, &HexSI, TRUE);
			if ((width > TextArea.left + ClientXGap + Hex.FontWidth) && (!Hex.TextView))
				r->right = r->left + TextArea.right + ClientXGap;
			else if ((width < TextArea.right + ClientXGap - Hex.FontWidth) && (Hex.TextView))
				r->right = r->left + TextArea.left + ClientXGap;
		}
		HexUpdateDialog();
		return 0;
		}
		break;

	case WM_EXITSIZEMOVE: {
		RECT r;
		GetWindowRect(hDlg, &r);
		if (r.right - r.left == TextArea.left + ClientXGap)
			Hex.TextView = 0;
		if (r.right - r.left == TextArea.right + ClientXGap)
			Hex.TextView = 1;
		HexUpdateDialog();
		}
		break;

	case WM_NCHITTEST: {
		LRESULT lRes = DefWindowProc(hDlg, uMsg, wParam, lParam);
		if (lRes == HTBOTTOMLEFT || lRes == HTTOPLEFT || lRes == HTTOPRIGHT ||
			lRes == HTTOP        || lRes == HTLEFT    || lRes == HTSIZE     )
			lRes = HTBORDER;
		return lRes;
		}
		break;

	case WM_GETMINMAXINFO: {
		MINMAXINFO *pInfo = (MINMAXINFO *) lParam;
		// Manual adjust to account for cell parameters
		pInfo->ptMinTrackSize.y = Hex.CellHeight * 2 + ClientTopGap;
		if (HexStarted) {
			pInfo->ptMinTrackSize.x = TextArea.left + ClientXGap;
			pInfo->ptMaxTrackSize.x = TextArea.right + ClientXGap; }
		return 0;
		}
		break;

	case WM_CLOSE:
		if (Full_Screen) {
			while (ShowCursor(true) < 0);
			while (ShowCursor(false) >= 0); }
		HexDestroyDialog();
		return 0;
		break;
	}
	return DefWindowProc(hDlg, uMsg, wParam, lParam); }

void HexCreateDialog() {
	WNDCLASSEX wndclass;
	if (!HexEditorHWnd) {
		memset(&wndclass, 0, sizeof(wndclass));
		wndclass.cbSize        = sizeof(WNDCLASSEX);
		wndclass.style         = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc   = HexEditorProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = ghInstance;
		wndclass.hIcon         = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS));
		wndclass.hIconSm       = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS));
		wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName  = "HEXEDITOR_MENU";
		wndclass.lpszClassName = "HEXEDITOR";
		if(!RegisterClassEx(&wndclass)) {
			Put_Info("Error Registering HEXEDITOR Window Class.");
			return; }
		HexFont = CreateFont(
			Hex.FontHeight, Hex.FontWidth,	// height, width
			0, 0, Hex.FontWeight,			// escapement, orientation, weight
			FALSE, FALSE, FALSE,			// italic, underline, strikeout
			ANSI_CHARSET, OUT_DEVICE_PRECIS,// charset, precision
			CLIP_MASK, DEFAULT_QUALITY,		// clipping, quality
			DEFAULT_PITCH, "Courier New" 	// pitch, name
		);
		HexEditorHWnd = CreateWindowEx(0, "HEXEDITOR", "RAM Dump",
			WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX | WS_VSCROLL,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, NULL, NULL, ghInstance, NULL);
		ShowWindow(HexEditorHWnd, SW_SHOW);
		HexUpdateCaption();
		DialogsOpen++;
	} else {
		ShowWindow(HexEditorHWnd, SW_SHOWNORMAL);
		SetForegroundWindow(HexEditorHWnd);
		HexUpdateCaption();
	}
}