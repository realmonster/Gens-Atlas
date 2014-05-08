// by r57shell

#ifndef M68K_DEBUG_WINDOW_H
#define M68K_DEBUG_WINDOW_H

#include "debugwindow.h"

struct M68kDebugWindow:DebugWindow
{
	M68kDebugWindow();

	uint32 last_pc;
	uint32 prev_pc;

	uint32 disrows[DEBUG_DISASM_ROWS];

	void Update();
	int DisasmLen(int pc);
	void DoStepOver();
	void TracePC(int pc);
	void TraceRead(uint32 start,uint32 stop);
	void TraceWrite(uint32 start,uint32 stop);
	bool IsShowedAddress(int pc);
	~M68kDebugWindow();
};

extern M68kDebugWindow M68kDW;

#endif