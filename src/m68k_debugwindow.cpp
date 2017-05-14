// by r57shell 

#include "m68k_debugwindow.h"
#include "Mem_M68k.h"
#include "Star_68k.h"
#include "M68KD.h"
#include "ram_dump.h"
#include "resource.h"

M68kDebugWindow M68kDW;

LRESULT CALLBACK M68kDebugProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return M68kDW.Proc(hDlg,uMsg,wParam,lParam);
}

M68kDebugWindow::M68kDebugWindow()
{
	ScrollMin=0;
	ScrollMax=(sizeof(Rom_Data))-RAM_DUMP_ROWS;
	dismap.resize((sizeof(Rom_Data)>>3)+1);
	DebugProc=(DLGPROC)M68kDebugProc;
	
	Title="M68k Debug";
}

M68kDebugWindow::~M68kDebugWindow()
{
}

extern "C"
{
	extern uint32 hook_pc;
}

extern uint32 Current_PC;

unsigned short Next_Word_T(void);
unsigned int Next_Long_T(void);

void M68kDebugWindow::TracePC(int pc)
{
    prev_pc=last_pc;
	last_pc=hook_pc;
	if (last_pc>=0&&
		(last_pc>>3)<dismap.size()
		)
		dismap[last_pc>>3]|=(1<<(last_pc&7));
	Current_PC=last_pc;
	int OPC = Next_Word_T();

	bool br=false;
	if (StepInto||StepOver==hook_pc)
	{
		br=true;
		if (StepInto)
			SetWhyBreak("StepInto");
		else
			SetWhyBreak("StepOver");
	}

	if (!br)
	{
		br=BreakPC(last_pc);
		if (br)
		{
			char bwhy[30];
			sprintf(bwhy,"Breakpoint PC:%06X",last_pc&0xFFFFFF);
			SetWhyBreak(bwhy);
		}
	}

	if (br)
	{
		StepInto=false;
		StepOver=-1;
		Breakpoint(last_pc);
	}
	//if ((OPC >> 12)==4&&!(OPC & 0x100)&&((OPC >> 6) & 0x3F)==58)
	//if((OPC >> 12)==6&&(OPC & 0xF00) == 0x100)
	if ((OPC&0xFFC0)==0x4E80||//jsr
		(OPC&0xFF00)==0x6100)//bsr
	{
		if (callstack.size()==100)
			callstack.erase(callstack.begin());
		callstack.push_back(last_pc);
	}

	//(OPC & 0x7)==5 && ((OPC >> 3) & 0x7)==6 && ((OPC >> 6) & 0x3F)==57 && !(OPC & 0x100) && (OPC >> 12)==4
	if ((OPC&0xFFFF)==0x4E75)//rts
	{
		if (!callstack.empty())
			callstack.pop_back();
	}
}

void M68kDebugWindow::TraceRead(uint32 start,uint32 stop)
{
	if (BreakRead(last_pc,start,stop))
	{
		char bwhy[33];
		sprintf(bwhy,"Read: %08X-%08X",start,stop);
		SetWhyBreak(bwhy);
		Breakpoint(last_pc);
	}
}

void M68kDebugWindow::TraceWrite(uint32 start,uint32 stop)
{
	if (BreakWrite(last_pc,start,stop))
	{
		char bwhy[33];
		sprintf(bwhy,"Write: %08X-%08X",start,stop);
		SetWhyBreak(bwhy);
		Breakpoint(last_pc);
	}
}

bool M68kDebugWindow::IsShowedAddress(int pc)
{
	int i;
	for (i=0;i<DEBUG_DISASM_ROWS;++i)
		if (disrows[i]==pc)
			break;
	return i<DEBUG_DISASM_ROWS;
}

void M68kDebugWindow::Update()
{
	if (!HWnd)
		return;
	RECT r;
	r.top=r.left=0;
	r.right=DEBUG_DISASM_WIDTH;
	r.bottom=DEBUG_DISASM_HEIGHT;
	FillRect(MemDC,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
	static char buff[2000];
	int i,x=GetScrollPos(GetDlgItem(HWnd,IDC_SCROLLBAR1),SB_CTL);
	Current_PC=GetNearestScroll(x);
	for (i=0;i<DEBUG_DISASM_ROWS;++i)
	{
		SetTextColor(MemDC,0x000000);
		sprintf( buff, "%06X",
			Current_PC & 0xffffff);
		disrows[i]=Current_PC & 0xffffff;
		TextOut(MemDC,5,i*18,buff,strlen(buff));
		if (Current_PC>=0&&
			(Current_PC>>3)<dismap.size()&&
			(dismap[Current_PC>>3]&(1<<(Current_PC&7)) )
			)
		{
			SetTextColor(MemDC,last_pc==Current_PC?0x00CC00:0xCC0000);
			sprintf( buff, "%-33s",
				M68KDisasm2( Next_Word_T, Next_Long_T, Current_PC ) );
			buff[7]=0;
			_strlwr(buff);
			TextOut(MemDC,60,i*18,buff,strlen(buff));
			SetTextColor(MemDC,0x000000);
			TextOut(MemDC,130,i*18,buff+8,strlen(buff+8));
		}
		else
		{
			int s=Next_Word_T();
			sprintf( buff, "%02X",
				s&0xff);
			--Current_PC;
			TextOut(MemDC,60,i*18,buff,strlen(buff));
		}
	}
	if (SelectedLine!=-1)
	{
		r.top=SelectedLine*18;
		r.bottom=(SelectedLine+1)*18-1;
		DrawFocusRect(MemDC,&r);
		SelectedPC=disrows[SelectedLine];
	}
	sprintf(buff,"A0: %08X  D0: %08X\n"
		"A1: %08X  D1: %08X\n"
		"A2: %08X  D2: %08X\n"
		"A3: %08X  D3: %08X\n"
		"A4: %08X  D4: %08X\n"
		"A5: %08X  D5: %08X\n"
		"A6: %08X  D6: %08X\n"
		"A7: %08X  D7: %08X\n"
		"Last PC: %08X\n"
		"%c%c%c%c%c"
		,main68k_context.areg[0],main68k_context.dreg[0]
		,main68k_context.areg[1],main68k_context.dreg[1]
		,main68k_context.areg[2],main68k_context.dreg[2]
		,main68k_context.areg[3],main68k_context.dreg[3]
		,main68k_context.areg[4],main68k_context.dreg[4]
		,main68k_context.areg[5],main68k_context.dreg[5]
		,main68k_context.areg[6],main68k_context.dreg[6]
		,main68k_context.areg[7],main68k_context.dreg[7]
		,prev_pc
		,(main68k_context.sr & 0x10)?'X':'x'
		,(main68k_context.sr & 0x08)?'N':'n'
		,(main68k_context.sr & 0x04)?'Z':'z'
		,(main68k_context.sr & 0x02)?'V':'v'
		,(main68k_context.sr & 0x01)?'C':'c');
	SetWindowText(GetDlgItem(HWnd,IDC_DEBUG_REGS), buff);
	SendDlgItemMessage(HWnd,IDC_CALL_STACK,LB_RESETCONTENT,NULL,NULL);
	for (i=0;(uint32)i<callstack.size();++i)
	{
		sprintf(buff,"%08X",callstack[i]);
		SendDlgItemMessage(HWnd,IDC_CALL_STACK,LB_INSERTSTRING,(WPARAM)(i?0:-1),(LPARAM)buff);
	}
	InvalidateRect(HWnd,NULL,FALSE);
}

void M68kDebugWindow::DoStepOver()
{
	Current_PC=last_pc;
	char *dis=M68KDisasm2( Next_Word_T, Next_Long_T, last_pc );
	if (strncmp(dis,"JSR",3)==0
	  ||strncmp(dis,"BSR",3)==0)
	  StepOver=Current_PC;
	else
	{
		StepInto=true;
		StepOver=-1;
	}
}

int M68kDebugWindow::DisasmLen(int pc)
{
	int x=Current_PC;
	Current_PC=pc;
	M68KDisasm2( Next_Word_T, Next_Long_T, Current_PC );
	int r=Current_PC-pc;
	Current_PC=x;
	return r;
}