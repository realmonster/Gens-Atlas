// by r57shell

#ifndef Z80_DEBUG_WINDOW_H
#define Z80_DEBUG_WINDOW_H

#include "debugwindow.h"

struct z80DebugWindow:DebugWindow
{
	z80DebugWindow();

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
	~z80DebugWindow();
};

extern z80DebugWindow z80DW;

#endif