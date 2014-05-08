#ifndef HEXEDITOR_H
#define HEXEDITOR_H

typedef struct {
	bool TextView, FontBold;
	unsigned int
		FontHeight, FontWidth, FontWeight,
		GapFontX, GapFontY, GapHeaderX, GapHeaderY,
		CellHeight, CellWidth,
		DialogPosX, DialogPosY,
		OffsetVisibleFirst, OffsetVisibleTotal,
		AddressSelectedFirst, AddressSelectedTotal, AddressSelectedLast,
		MemoryRegion;
	COLORREF
		ColorFont, ColorBG, ColorSelection;
} HexParameters;
/*
typedef struct {
	char Name[12];
	unsigned char* Array;
	unsigned int
		Offset,
		Size,
		RowCount;
} HexRegion;
*/
enum MousePos {
	NO,
	CELL,
	TEXT
};

extern HWND HexEditorHWnd;
extern HexParameters Hex;
void HexCreateDialog();
void HexDestroyDialog();
void HexUpdateDialog(bool ClearBG = 0);
extern bool DrawLines;

#endif