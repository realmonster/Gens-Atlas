#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <vector>

#define u8 UINT8

struct HexRegion {
	char Name[12];
	u8*  Array;
	UINT Offset;
	UINT Size;
	bool Active;
	u8   Swap;
};

enum MousePos {
	NO,
	CELL,
	TEXT
};

struct HexParams {
	HWND Hwnd;
	HDC  DC;
	UINT InstanceLimit;
	char InputDigit;
	bool
		MultiInstance, MouseButtonHeld, SecondDigitPrompted, Running,
		TextView, DrawLines, FontBold;
	UINT
		FontHeight, FontWidth, FontWeight,
		Gap, GapHeaderX, GapHeaderY,
		CellHeight, CellWidth,
		DialogPosX, DialogPosY,
		OffsetVisibleFirst, OffsetVisibleTotal,
		AddressSelectedFirst, AddressSelectedTotal, AddressSelectedLast;
	COLORREF
		ColorFont, ColorBG;
	HexRegion CurrentRegion;
	MousePos MouseArea;
	SCROLLINFO SI;
};

struct SymbolName {
	u8*  Array;
	UINT Start;
	UINT Size;
	char*Name;
};

struct HardPatch {
	u8*  Array;
	UINT Address;
	u8   Value;
	bool Active;
};

void HexCreateDialog();
void HexDestroyDialog(HexParams *Hex);
void HexUpdateDialog(HexParams *Hex, bool ClearBG = 0);
void HexWatchPoint68K(UINT Start, UINT Size);
void HexWatchPointZ80(UINT Start, UINT Size);
extern std::vector<HexParams *> HexEditors;
extern HexParams HexCommon;

#endif