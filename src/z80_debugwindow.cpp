// by r57shell

#include "z80_debugwindow.h"
#include "Mem_z80.h"
#include "Mem_M68k.h"
#include "z80.h"
#include "z80dis.h"
//#include "Star_68k.h"
#include "ram_dump.h"
#include "resource.h"
#include "luascript.h"

z80DebugWindow z80DW;

LRESULT CALLBACK z80DebugProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return z80DW.Proc(hDlg,uMsg,wParam,lParam);
}

z80DebugWindow::z80DebugWindow()
{
	ScrollMin=0;
	ScrollMax=(0x4000)-RAM_DUMP_ROWS;
	dismap.resize((0x4000>>3)+1);
	DebugProc=(DLGPROC)z80DebugProc;
	
	Title="z80 Debug";
}

z80DebugWindow::~z80DebugWindow()
{
}

extern "C" {
	void __fastcall z80TracePC(unsigned int pc);
	void __fastcall z80TraceRead(uint32 start,uint32 size);
	void __fastcall z80TraceWrite(uint32 start,uint32 size);
}

void __fastcall z80TracePC(unsigned int pc)
{
	z80DW.TracePC(pc);
	CallRegisteredLuaMemHook(pc, 1, 0, LUAMEMHOOK_EXEC_Z80);
}

void __fastcall z80TraceRead(uint32 start,uint32 size)
{
	z80DW.TraceRead(start,start+size-1);
	CallRegisteredLuaMemHook(start, size, 0, LUAMEMHOOK_READ_Z80);
}

void __fastcall z80TraceWrite(uint32 start,uint32 size)
{
	z80DW.TraceWrite(start,start+size-1);
	CallRegisteredLuaMemHook(start, size, 0, LUAMEMHOOK_WRITE_Z80);
}

void z80DebugWindow::TracePC(int pc)
{
    prev_pc=last_pc;
	last_pc=pc;
	if (last_pc>=0&&
		(last_pc>>3)<dismap.size()
		)
		dismap[last_pc>>3]|=(1<<(last_pc&7));
	
	bool br=false;
	if (StepInto||StepOver==pc)
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
	//if ((OPC&0xFFC0)==0x4E80||//jsr
	//	(OPC&0xFF00)==0x6100)//bsr
	//	callstack.push_back(last_pc);

	//(OPC & 0x7)==5 && ((OPC >> 3) & 0x7)==6 && ((OPC >> 6) & 0x3F)==57 && !(OPC & 0x100) && (OPC >> 12)==4
	//if ((OPC&0xFFFF)==0x4E75)//rts
	//	callstack.pop_back();
}

void z80DebugWindow::TraceRead(uint32 start,uint32 stop)
{
	if (BreakRead(last_pc,start,stop))
	{
		char bwhy[33];
		sprintf(bwhy,"Read: %08X-%08X",start,stop);
		SetWhyBreak(bwhy);
		Breakpoint(last_pc);
	}
}

void z80DebugWindow::TraceWrite(uint32 start,uint32 stop)
{
	if (BreakWrite(last_pc,start,stop))
	{
		char bwhy[33];
		sprintf(bwhy,"Write: %08X-%08X",start,stop);
		SetWhyBreak(bwhy);
		Breakpoint(last_pc);
	}
}

bool z80DebugWindow::IsShowedAddress(int pc)
{
	int i;
	for (i=0;i<DEBUG_DISASM_ROWS;++i)
		if (disrows[i]==pc)
			break;
	return i<DEBUG_DISASM_ROWS;
}

void z80DebugWindow::Update()
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
	int pc=GetNearestScroll(x);
	for (i=0;i<DEBUG_DISASM_ROWS;++i)
	{
		SetTextColor(MemDC,0x000000);
		sprintf( buff, "%04X:",
			pc & 0x3fff);
		disrows[i]=pc & 0x3fff;
		TextOut(MemDC,5,i*18,buff,strlen(buff));
		if (pc>=0&&
			(pc>>3)<dismap.size()&&
			(dismap[pc>>3]&(1<<(pc&7)) )
			)
		{
			SetTextColor(MemDC,last_pc==pc?0x00CC00:0xCC0000);
			z80dis(Ram_Z80, &pc, buff);
			TextOut(MemDC,60,i*18,buff+6,strlen(buff+6)-1);
			SetTextColor(MemDC,0x000000);
		}
		else
		{
			int s=Ram_Z80[pc];
			sprintf( buff, "%02X",
				s&0xff);
			++pc;
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
	sprintf(buff,              "AF =%.4X BC =%.4X DE =%.4X\n",
		M_Z80.AF.w.AF&0xFFFF, M_Z80.BC.w.BC&0xFFFF, M_Z80.DE.w.DE&0xFFFF);
	sprintf(buff+strlen(buff), "AF2=%.4X BC2=%.4X DE2=%.4X\n",
		M_Z80.AF2.w.AF2&0xFFFF, M_Z80.BC2.w.BC2&0xFFFF, M_Z80.DE2.w.DE2&0xFFFF);
	sprintf(buff+strlen(buff), "IX =%.4X IY =%.4X SP =%.4X\n",
		M_Z80.IX.w.IX&0xFFFF, M_Z80.IY.w.IY&0xFFFF, M_Z80.SP.w.SP&0xFFFF);
	sprintf(buff+strlen(buff), "HL =%.4X HL2=%.4X PC =%.4X\n",
		M_Z80.HL.w.HL&0xFFFF, M_Z80.HL2.w.HL2&0xFFFF, M_Z80.PC.w.PC&0xFFFF);
	sprintf(buff+strlen(buff), "IFF1=%d IFF2=%d I=%.2X R=%.2X IM=%.2X\n",
		M_Z80.IFF.b.IFF1, M_Z80.IFF.b.IFF2, M_Z80.I, M_Z80.R.b.R1, M_Z80.IM);
	sprintf(buff+strlen(buff), "%c%c%c%c%c%c%c%c\n",
		(M_Z80.AF.w.AF & 0x80)?'S':'s',
		(M_Z80.AF.w.AF & 0x40)?'Z':'z',
		(M_Z80.AF.b.FXY & 0x20)?'Y':'y',
		(M_Z80.AF.w.AF & 0x10)?'H':'h',
		(M_Z80.AF.b.FXY & 0x08)?'X':'x',
		(M_Z80.AF.w.AF & 0x04)?'P':'p',
		(M_Z80.AF.w.AF & 0x02)?'N':'n',
		(M_Z80.AF.w.AF & 0x01)?'C':'c');
	sprintf(buff+strlen(buff), "Status=%.2X ILine=%.2X IVect=%.2X\n", M_Z80.Status & 0xFF, M_Z80.IntLine, M_Z80.IntVect);
	sprintf(buff+strlen(buff), "Bank68K=%.8X State=%.2X\n", Bank_M68K, Z80_State);

	SetWindowText(GetDlgItem(HWnd,IDC_DEBUG_REGS), buff);
	SendDlgItemMessage(HWnd,IDC_CALL_STACK,LB_RESETCONTENT,NULL,NULL);
	for (i=0;i<callstack.size();++i)
	{
		sprintf(buff,"%08X",callstack[i]);
		SendDlgItemMessage(HWnd,IDC_CALL_STACK,LB_INSERTSTRING,(WPARAM)(i?0:-1),(LPARAM)buff);
	}
	InvalidateRect(HWnd,NULL,FALSE);
}

void z80DebugWindow::DoStepOver()
{
	int pc=last_pc;
	char ch[128];
	z80dis(Ram_Z80, &pc, ch);
	if (strncmp(ch+18,"CALL",4)==0)
	  StepOver=pc;
	else
	{
		StepInto=true;
		StepOver=-1;
	}
}

int z80DebugWindow::DisasmLen(int pc)
{
	int lpc=pc;
	char ch[128];
	z80dis(Ram_Z80, &pc, ch);
	return pc-lpc;
}