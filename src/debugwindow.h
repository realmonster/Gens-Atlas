// by r57shell

#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H
#include <windows.h>
#include <vector>
#include <string>

#define WM_DEBUG_DUMMY_EXIT (WM_USER+1000)

#define uint32 unsigned int
#define uint8 unsigned char

struct Breakpoint
{
	bool enabled;
	uint32 start;
	uint32 end;
	uint8 type;
};

struct DebugWindow
{
	DebugWindow();
	std::vector<uint32> callstack;
	std::vector<Breakpoint> Breakpoints;
	std::vector<uint8> dismap;
	std::string whyBreakpoint;

	char * Title;
	DLGPROC DebugProc;
	HFONT Fonts[5];
	HWND HWnd;
	HWND DummyHWnd;
	HDC MemDC;
	HBITMAP MemBMP;
	HBITMAP LastBMP;
	int ScrollMin,ScrollMax;

	bool StepInto;
	bool IdaSync;
	uint32 StepOver;
	int SelectedLine;
	int SelectedPC;

	void Window();
	void UpdateBreak(int n);
	void ShowAddress(int pc);
	void SetDisasmPos(int x);
	int GetNearestScroll(int x);
	LRESULT CALLBACK Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Breakpoint(int pc);
	void SetWhyBreak(LPCSTR lpString);

	bool BreakPC(int pc);
	bool BreakRead(int pc,uint32 start,uint32 stop);
	bool BreakWrite(int pc,uint32 start,uint32 stop);

	void ResetMap();
	void LoadMap(char *fname);
	void SaveMap(char *fname);

	virtual void Update();
	virtual int DisasmLen(int pc);
	virtual void DoStepOver();
	virtual void TracePC(int pc);
	virtual void TraceRead(uint32 start,uint32 stop);
	virtual void TraceWrite(uint32 start,uint32 stop);
	virtual bool IsShowedAddress(int pc);
	virtual ~DebugWindow();
};

#define DEBUG_DISASM_ROWS 26
#define DEBUG_DISASM_WIDTH 375
#define DEBUG_DISASM_HEIGHT (DEBUG_DISASM_ROWS)*18

char *GetMsgName(int x);
void IdaGo(int dest);

#endif

