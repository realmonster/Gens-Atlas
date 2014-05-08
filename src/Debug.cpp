#include "gens.h"
#ifdef GENS_DEBUG

#include <windows.h>
#include <stdio.h>

#include "G_main.h"
#include "G_Input.h"
#include "keycode.h"
#include "misc.h"
#include "gens.h"

#include "Debug.h"
#include "Cpu_68k.h"
#include "Star_68k.h"
#include "M68KD.h"
#include "Cpu_z80.h"
#include "z80.h"
#include "z80dis.h"
#include "Cpu_SH2.h"
#include "SH2.h"
#include "SH2D.h"
#include "mem_M68K.h"
#include "mem_S68K.h"
#include "mem_Z80.h"
#include "mem_SH2.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32X.h"
#include "LC89510.h"
#include "cd_aspi.h"
#include "gfx_cd.h"

int Current_32X_FB = 0;
int adr_mem = 0, nb_inst = 1, cd_pattern_adr = 0,pattern_adr = 0, pattern_pal;
int Current_PC;
char GString[1024];
char Dbg_Str[32];
char Dbg_EA_Str[16];
char Dbg_Size_Str[3];
char Dbg_Cond_Str[3];


void Debug_Event(int key)
{
	int i;
	SH2_CONTEXT *sh;

	if (Debug & 1) sh = &M_SH2;
	else sh = &S_SH2;

	switch(key)
	{
		case K_T:
			if ((Debug == 1) || (Debug == 3))
			{
				main68k_tripOdometer();
				main68k_exec(1);
			}
			else if (Debug == 2)
			{
				z80_Clear_Odo(&M_Z80);
				z80_Exec(&M_Z80, 1);
			}
			else if ((Debug >= 4) && (Debug < 7))
			{
				sub68k_tripOdometer();
				sub68k_exec(1);
			}
			else if ((Debug >= 7) && (Debug < 10))
			{
				SH2_Clear_Odo(sh);
				SH2_Exec(sh, 1);
			}
			break;

		case K_Y:
			for(i = 0; i < 10; i++)
			{
				if ((Debug == 1) || (Debug == 3))
				{
					main68k_tripOdometer();
					main68k_exec(1);
				}
				else if (Debug == 2)
				{
					z80_Clear_Odo(&M_Z80);
					z80_Exec(&M_Z80, 1);
				}
				else if ((Debug >= 4) && (Debug < 7))
				{
					sub68k_tripOdometer();
					sub68k_exec(1);
				}
				else if ((Debug >= 7) && (Debug < 10))
				{
					SH2_Clear_Odo(sh);
					SH2_Exec(sh, 1);
				}
			}
			break;

		case K_U:
			for(i = 0; i < 100; i++)
			{
				if ((Debug == 1) || (Debug == 3))
				{
					main68k_tripOdometer();
					main68k_exec(1);
				}
				else if (Debug == 2)
				{
					z80_Clear_Odo(&M_Z80);
					z80_Exec(&M_Z80, 1);
				}
				else if ((Debug >= 4) && (Debug < 7))
				{
					sub68k_tripOdometer();
					sub68k_exec(1);
				}
				else if ((Debug >= 7) && (Debug < 10))
				{
					SH2_Clear_Odo(sh);
					SH2_Exec(sh, 1);
				}
			}
			break;

		case K_I:
			for(i = 0; i < 1000; i++)
			{
				if ((Debug == 1) || (Debug == 3))
				{
					main68k_tripOdometer();
					main68k_exec(1);
				}
				else if (Debug == 2)
				{
					z80_Clear_Odo(&M_Z80);
					z80_Exec(&M_Z80, 1);
				}
				else if ((Debug >= 4) && (Debug < 7))
				{
					sub68k_tripOdometer();
					sub68k_exec(1);
				}
				else if ((Debug >= 7) && (Debug < 10))
				{
					SH2_Clear_Odo(sh);
					SH2_Exec(sh, 1);
				}
			}
			break;

		case K_O:
			for(i = 0; i < 10000; i++)
			{
				if ((Debug == 1) || (Debug == 3))
				{
					main68k_tripOdometer();
					main68k_exec(1);
				}
				else if (Debug == 2)
				{
					z80_Clear_Odo(&M_Z80);
					z80_Exec(&M_Z80, 1);
				}
				else if ((Debug >= 4) && (Debug < 7))
				{
					sub68k_tripOdometer();
					sub68k_exec(1);
				}
				else if ((Debug >= 7) && (Debug < 10))
				{
					SH2_Clear_Odo(sh);
					SH2_Exec(sh, 1);
				}
			}
			break;

		case K_P:
			for(i = 0; i < 100000; i++)
			{
				if ((Debug == 1) || (Debug == 3))
				{
					main68k_tripOdometer();
					main68k_exec(1);
				}
				else if (Debug == 2)
				{
					z80_Clear_Odo(&M_Z80);
					z80_Exec(&M_Z80, 1);
				}
				else if ((Debug >= 4) && (Debug < 7))
				{
					sub68k_tripOdometer();
					sub68k_exec(1);
				}
				else if ((Debug >= 7) && (Debug < 10))
				{
					SH2_Clear_Odo(sh);
					SH2_Exec(sh, 1);
				}
			}
			break;

		case K_Z:
			adr_mem -= 0xC * 0xC * 0xC;
			break;

		case K_S:
			adr_mem += 0xC * 0xC * 0xC;
			break;

		case K_E:
			adr_mem -= 0xC * 0xC;
			break;

		case K_D:
			adr_mem += 0xC * 0xC;
			break;

		case K_R:
			adr_mem -= 0xC;
			break;

		case K_F:
			adr_mem += 0xC;
			break;

		case K_H:
			if (Debug == 1) main68k_interrupt(4, -1);
			else if (Debug == 2) z80_Interrupt(&M_Z80, 0xFF);
			else if ((Debug >= 4) && (Debug < 7)) sub68k_interrupt(5, -1);
			else if ((Debug >= 7) && (Debug < 9)) SH2_Interrupt(sh, 8);
			break;

		case K_J:
			if (Debug == 1) main68k_interrupt(6, -1);
			else if (Debug == 2) z80_Interrupt(&M_Z80, 0xFF);
			else if ((Debug >= 4) && (Debug < 7)) sub68k_interrupt(4, -1);
			else if ((Debug >= 7) && (Debug < 9)) SH2_Interrupt(sh, 12);
			break;

		case K_L:
			if (VDP_Current_Line >  0) VDP_Current_Line--;
			break;

		case K_M:
			if (VDP_Current_Line <  319) VDP_Current_Line++;
			break;

		case K_X:
			Debug ^= 0x0100;
			break;
				
		case K_C:
			if ((Debug > 3) && (Debug < 6)) SCD.Cur_LBA++;
			VDP_Status ^= 0x8;
			break;

		case K_V:
			VDP_Status ^= 0x4;
			Current_32X_FB ^= 1;
			break;

		case K_N:
			if ((Debug > 6) && (Debug < 9))
			{
				sh->PC += 2;
				sh->Status &= 0xFFFFFFF0;
			}
			else if ((Debug > 3) && (Debug < 6))
			{
				sub68k_context.pc += 2;
			}
			else if (Debug == 1)
			{
				main68k_context.pc += 2;
			}
			else if (Debug == 2)
			{
				z80_Set_PC(&M_Z80, z80_Get_PC(&M_Z80) + 1);
			}
			break;

		case K_W:
			if ((Debug > 3) && (Debug < 6)) Check_CD_Command();

		case K_SPACE:
			if (Debug) 
			{
				Debug++;

				if (SegaCD_Started)
				{
					if (Debug > 6) Debug = 1;
				}
				else if (_32X_Started)
				{
					if ((Debug > 3) && (Debug < 7)) Debug = 7;
					if (Debug > 9) Debug = 1;
				}
				else if (Debug > 3) Debug = 1;

				Build_Main_Menu();
			}
			break;

		case K_DIV:
			VDP_Status &= ~2;
			break;

		case K_ETOILE:
			pattern_pal++;
			pattern_pal &= 0xF;
			break;

		case K_PLUS:
			if (pattern_adr < 0xD000) pattern_adr = (pattern_adr + 0x200) & 0xFFFF;
			if (SegaCD_Started)
				if (cd_pattern_adr < 0x3D000) cd_pattern_adr = (cd_pattern_adr + 0x800) & 0x3FFFF;
			break;

		case K_MINUS:
			if (pattern_adr > 0) pattern_adr = max(0,(pattern_adr - 0x200));
			if (SegaCD_Started)
				if (cd_pattern_adr > 0)
					cd_pattern_adr = max(0,cd_pattern_adr - 0x800);
			break;
	}
}

			
unsigned short Next_Word(void)
{
	unsigned short val;
	
	if (Debug == 1) val = M68K_RW(Current_PC);
	else if (Debug >= 2) val = S68K_RW(Current_PC);

	Current_PC += 2;

	return(val);
}


unsigned int Next_Long(void)
{
	unsigned int val;
	
	if (Debug == 1)
	{
		val = M68K_RW(Current_PC);
		val <<= 16;
		val |= M68K_RW(Current_PC + 2);
	}
	else if (Debug >= 2)
	{
		val = S68K_RW(Current_PC);
		val <<= 16;
		val |= S68K_RW(Current_PC + 2);
	}

	Current_PC += 4;

	return(val);
}


void Refresh_M68k_Inst(void)
{
	unsigned int i, PC;

	sprintf(Str_Tmp,"** MAIN 68000 DEBUG **");
	Print_Text(Str_Tmp, 22, 24, 1, GREEN);
	

	Current_PC = main68k_context.pc;
		
	for(i = 1; i < 14; i++)
	{
		PC = Current_PC;
		sprintf(GString, "%.4X   %-33s\n", PC, M68KDisasm(Next_Word, Next_Long));
		if (i == 1) Print_Text(GString, 39, 1, (i << 3) + 5, RED);
		else  Print_Text(GString, 39, 1, (i << 3) + 5, WHITE);
	}
}


void Refresh_S68k_Inst(void)
{
	unsigned int i, PC;

	sprintf(Str_Tmp,"** SUB 68000 DEBUG **");
	Print_Text(Str_Tmp, 22, 24, 1, GREEN);

	Current_PC = sub68k_context.pc;
		
	for(i = 1; i < 14; i++)
	{
		PC = Current_PC;
		sprintf(GString, "%.4X   %-33s\n", PC, M68KDisasm(Next_Word, Next_Long));
		if (i == 1) Print_Text(GString, 39, 1, (i << 3) + 5, RED);
		else  Print_Text(GString, 39, 1, (i << 3) + 5, WHITE);
	}
}


void Refresh_Z80_Inst(void)
{
	unsigned int i, PC;

	sprintf(Str_Tmp,"***** Z80 DEBUG *****");
	Print_Text(Str_Tmp, 22, 24, 1, GREEN);

	PC = z80_Get_PC(&M_Z80);
		
	for(i = 1; i < 14; i++)
	{
		z80dis((unsigned char *)Ram_Z80, (int *)&PC, GString);
		if (i == 1) Print_Text(GString, 39, 1, (i << 3) + 5, RED);
		else Print_Text(GString, 39, 1, (i << 3) + 5, WHITE);
	}
}


void Refresh_SH2_Inst(int num)
{
	unsigned int i, PC;
	SH2_CONTEXT *sh;

	if (num)
	{
		sprintf(Str_Tmp,"** SLAVE SH2 DEBUG **");
		Print_Text(Str_Tmp, 21, 24, 1, GREEN);
		sh = &S_SH2;
	}
	else
	{
		sprintf(Str_Tmp,"** MASTER SH2 DEBUG **");
		Print_Text(Str_Tmp, 22, 24, 1, GREEN);
		sh = &M_SH2;
	}

	PC = (sh->PC - sh->Base_PC) - 4;
		
	for(i = 1; i < 14; i++, PC += 2)
	{
		SH2Disasm(GString, PC, SH2_Read_Word(sh, PC), 0);
		if (i == 1) Print_Text(GString, 39, 1, (i << 3) + 5, RED);
		else Print_Text(GString, 39, 1, (i << 3) + 5, WHITE);
	}
}


void Refresh_M68k_Mem(void)
{
	unsigned int i, j, k, Adr; 

	Adr = adr_mem >> 1;
	sprintf(Str_Tmp,"** MAIN 68000 MEM **");
	Print_Text(Str_Tmp, 20, 24, 130, GREEN);

	for(k = 0, j = Adr; k < 7; k++, j+= 6)
	{
		i = (j & 0x7FFF) << 1;
		sprintf(GString, "%.4X:%.4X %.4X %.4X %.4X %.4X %.4X\n", i, Ram_68k[i] + (Ram_68k[i + 1] << 8), Ram_68k[i + 2] + (Ram_68k[i + 3] << 8), Ram_68k[i + 4] + (Ram_68k[i + 5] << 8), Ram_68k[i + 6] + (Ram_68k[i + 7] << 8), Ram_68k[i + 8] + (Ram_68k[i + 9] << 8), Ram_68k[i + 10] + (Ram_68k[i + 11] << 8));
		Print_Text(GString, 34, 1, 146 + (k << 3), WHITE);
	}
}


void Refresh_S68k_Mem(void)
{
	unsigned int i, j, k, Adr; 

	Adr = adr_mem >> 1;
	sprintf(Str_Tmp,"** SUB 68000 MEM **");
	Print_Text(Str_Tmp, 19, 24, 130, GREEN);

	for(k = 0, j = Adr; k < 7; k++, j+= 6)
	{
//		i = (j & 0x3FFFF) << 1;
//		sprintf(GString, "%.5X:%.4X %.4X %.4X %.4X %.4X %.4X\n", i, Ram_Prg[i] + (Ram_Prg[i + 1] << 8), Ram_Prg[i + 2] + (Ram_Prg[i + 3] << 8), Ram_Prg[i + 4] + (Ram_Prg[i + 5] << 8), Ram_Prg[i + 6] + (Ram_Prg[i + 7] << 8), Ram_Prg[i + 8] + (Ram_Prg[i + 9] << 8), Ram_Prg[i + 10] + (Ram_Prg[i + 11] << 8));
		i = (j & 0x1FFFF) << 1;
		sprintf(GString, "%.5X:%.4X %.4X %.4X %.4X %.4X %.4X\n", i, Ram_Word_1M[i] + (Ram_Word_1M[i + 1] << 8), Ram_Word_1M[i + 2] + (Ram_Word_1M[i + 3] << 8), Ram_Word_1M[i + 4] + (Ram_Word_1M[i + 5] << 8), Ram_Word_1M[i + 6] + (Ram_Word_1M[i + 7] << 8), Ram_Word_1M[i + 8] + (Ram_Word_1M[i + 9] << 8), Ram_Word_1M[i + 10] + (Ram_Word_1M[i + 11] << 8));
		Print_Text(GString, 35, 1, 146 + (k << 3), WHITE);
	}
}


void Refresh_Z80_Mem(void)
{
	unsigned int j, k; 

	sprintf(Str_Tmp,"***** Z80 MEM *****");
	Print_Text(Str_Tmp, 19, 24, 130, GREEN);

	for(k = 0, j = adr_mem & 0xFFFF; k < 7; k++, j = (j + 12) & 0xFFFF)
	{
		sprintf(GString, "%.4X:%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n", j, Z80_ReadB(j + 0), Z80_ReadB(j + 1), Z80_ReadB(j + 2), Z80_ReadB(j + 3), Z80_ReadB(j + 4), Z80_ReadB(j + 5), Z80_ReadB(j + 6), Z80_ReadB(j + 7), Z80_ReadB(j + 8), Z80_ReadB(j + 9), Z80_ReadB(j + 10), Z80_ReadB(j + 11));
		Print_Text(GString, 35, 1, 146 + (k << 3), WHITE);
	}
}


void Refresh_SH2_Mem(void)
{
	unsigned int i, j, k, Adr; 

	Adr = adr_mem >> 1;

	sprintf(Str_Tmp,"*** SH2 CPU MEM ***");
	Print_Text(Str_Tmp, 19, 24, 130, GREEN);

	for(k = 0, j = Adr; k < 7; k++, j+= 6)
	{
		i = (j & 0x1FFFF) << 1;
		sprintf(GString, "%.5X:%.4X %.4X %.4X %.4X %.4X %.4X\n", i, _32X_Ram[i] + (_32X_Ram[i + 1] << 8), _32X_Ram[i + 2] + (_32X_Ram[i + 3] << 8), _32X_Ram[i + 4] + (_32X_Ram[i + 5] << 8), _32X_Ram[i + 6] + (_32X_Ram[i + 7] << 8), _32X_Ram[i + 8] + (_32X_Ram[i + 9] << 8), _32X_Ram[i + 10] + (_32X_Ram[i + 11] << 8));
		Print_Text(GString, 35, 1, 146 + (k << 3), WHITE);
	}
}


void Refresh_M68k_State(void)
{
	sprintf(Str_Tmp,"** MAIN 68000 STATUS **");
	Print_Text(Str_Tmp, 23, 196, 130, GREEN);
	
	sprintf(GString, "A0=%.8X A1=%.8X A2=%.8X X=%d\n", main68k_context.areg[0], main68k_context.areg[1], main68k_context.areg[2], (main68k_context.sr & 0x10)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 146, WHITE);
	sprintf(GString, "A3=%.8X A4=%.8X A5=%.8X N=%d\n", main68k_context.areg[3], main68k_context.areg[4], main68k_context.areg[5], (main68k_context.sr & 0x8)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 154, WHITE);
	sprintf(GString, "A6=%.8X A7=%.8X D0=%.8X Z=%d\n", main68k_context.areg[6], main68k_context.areg[7], main68k_context.dreg[0], (main68k_context.sr & 0x4)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 162, WHITE);
	sprintf(GString, "D1=%.8X D2=%.8X D3=%.8X V=%d\n", main68k_context.dreg[1], main68k_context.dreg[2], main68k_context.dreg[3], (main68k_context.sr & 0x2)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 170, WHITE);
	sprintf(GString, "D4=%.8X D5=%.8X D6=%.8X C=%d\n", main68k_context.dreg[4], main68k_context.dreg[5], main68k_context.dreg[6], (main68k_context.sr & 0x1)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 178, WHITE);
	sprintf(GString, "D7=%.8X PC=%.8X SR=%.4X\n", main68k_context.dreg[7], main68k_context.pc, main68k_context.sr);
	Print_Text(GString, strlen(GString) - 1, 162, 186, WHITE);
	sprintf(GString, "Cycles=%.10d \n", main68k_context.odometer);
	Print_Text(GString, strlen(GString) - 1, 162, 194, WHITE);
	sprintf(GString, "Bank for Z80 = %.8X\n", Bank_Z80);
	Print_Text(GString, strlen(GString) - 1, 162, 202, WHITE);
//	sprintf(GString, "Bank = %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n", Rom_Bank[0], Rom_Bank[1], Rom_Bank[2], Rom_Bank[3], Rom_Bank[4], Rom_Bank[5], Rom_Bank[6], Rom_Bank[7]);
//	Print_Text(GString, strlen(GString) - 1, 162, 210, WHITE);
}


void Refresh_S68k_State(void)
{
	sprintf(Str_Tmp,"** SUB 68000 STATUS **");
	Print_Text(Str_Tmp, 22, 196, 130, GREEN);

	sprintf(GString, "A0=%.8X A1=%.8X A2=%.8X X=%d\n", sub68k_context.areg[0], sub68k_context.areg[1], sub68k_context.areg[2], (sub68k_context.sr & 0x10)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 146, WHITE);
	sprintf(GString, "A3=%.8X A4=%.8X A5=%.8X N=%d\n", sub68k_context.areg[3], sub68k_context.areg[4], sub68k_context.areg[5], (sub68k_context.sr & 0x8)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 154, WHITE);
	sprintf(GString, "A6=%.8X A7=%.8X D0=%.8X Z=%d\n", sub68k_context.areg[6], sub68k_context.areg[7], sub68k_context.dreg[0], (sub68k_context.sr & 0x4)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 162, WHITE);
	sprintf(GString, "D1=%.8X D2=%.8X D3=%.8X V=%d\n", sub68k_context.dreg[1], sub68k_context.dreg[2], sub68k_context.dreg[3], (sub68k_context.sr & 0x2)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 170, WHITE);
	sprintf(GString, "D4=%.8X D5=%.8X D6=%.8X C=%d\n", sub68k_context.dreg[4], sub68k_context.dreg[5], sub68k_context.dreg[6], (sub68k_context.sr & 0x1)?1:0);
	Print_Text(GString, strlen(GString) - 1, 162, 178, WHITE);
	sprintf(GString, "D7=%.8X PC=%.8X SR=%.4X\n", sub68k_context.dreg[7], sub68k_context.pc, sub68k_context.sr);
	Print_Text(GString, strlen(GString) - 1, 162, 186, WHITE);
	sprintf(GString, "Cycles=%.10d \n", sub68k_context.odometer);
	Print_Text(GString, strlen(GString) - 1, 162, 194, WHITE);
//	sprintf(GString, "Bank for main 68K = %.8X\n", Bank_M68K);
//	Print_Text(GString, strlen(GString) - 1, 162, 202, WHITE);
	sprintf(GString, "Bank for main 68K = %.8X\n", Bank_M68K);
	Print_Text(GString, strlen(GString) - 1, 162, 202, WHITE);
}


void Refresh_Z80_State(void)
{
	sprintf(Str_Tmp,"***** Z80 STATUS *****");
	Print_Text(Str_Tmp, 22, 196, 130, GREEN);

	sprintf(GString, "AF =%.4X BC =%.4X DE =%.4X HL =%.4X\n", z80_Get_AF(&M_Z80), M_Z80.BC.w.BC, M_Z80.DE.w.DE, M_Z80.HL.w.HL);
	Print_Text(GString, strlen(GString) - 1, 162, 146, WHITE);
	sprintf(GString, "AF2=%.4X BC2=%.4X DE2=%.4X HL2=%.4X\n", z80_Get_AF2(&M_Z80), M_Z80.BC2.w.BC2, M_Z80.DE2.w.DE2, M_Z80.HL2.w.HL2);
	Print_Text(GString, strlen(GString) - 1, 162, 154, WHITE);
	sprintf(GString, "IX =%.4X IY =%.4X SP =%.4X PC =%.4X\n", M_Z80.IX.w.IX, M_Z80.IY.w.IY, M_Z80.SP.w.SP, z80_Get_PC(&M_Z80));
	Print_Text(GString, strlen(GString) - 1, 162, 162, WHITE);
	sprintf(GString, "IFF1=%d IFF2=%d I=%.2X R=%.2X IM=%.2X\n", M_Z80.IFF.b.IFF1, M_Z80.IFF.b.IFF2, M_Z80.I, M_Z80.R.b.R1, M_Z80.IM);
	Print_Text(GString, strlen(GString) - 1, 162, 170, WHITE);
	sprintf(GString, "S=%d Z=%d Y=%d H=%d X=%d P=%d N=%d C=%d\n", (z80_Get_AF(&M_Z80) & 0x80) >> 7, (z80_Get_AF(&M_Z80) & 0x40) >> 6, (z80_Get_AF(&M_Z80) & 0x20) >> 5, (z80_Get_AF(&M_Z80) & 0x10) >> 4, (z80_Get_AF(&M_Z80) & 0x08) >> 3, (z80_Get_AF(&M_Z80) & 0x04) >> 2, (z80_Get_AF(&M_Z80) & 0x02) >> 1, (z80_Get_AF(&M_Z80) & 0x01) >> 0);
	Print_Text(GString, strlen(GString) - 1, 162, 178, WHITE);
	sprintf(GString, "Status=%.2X ILine=%.2X IVect=%.2X\n", M_Z80.Status & 0xFF, M_Z80.IntLine, M_Z80.IntVect);
	Print_Text(GString, strlen(GString) - 1, 162, 186, WHITE);
	sprintf(GString, "Bank68K=%.8X State=%.2X\n", Bank_M68K, Z80_State);
	Print_Text(GString, strlen(GString) - 1, 162, 194, WHITE);
}


void Refresh_SH2_State(int num)
{
	SH2_CONTEXT *sh;

	if (num)
	{
		sprintf(Str_Tmp,"** SLAVE SH2 STATUS **");
		Print_Text(Str_Tmp, 22, 196, 130, GREEN);
		sh = &S_SH2;
	}
	else
	{
		sprintf(Str_Tmp,"** MASTER SH2 STATUS **");
		Print_Text(Str_Tmp, 22, 196, 130, GREEN);
		sh = &M_SH2;
	}

	sprintf(GString, "R0=%.8X R1=%.8X R2=%.8X T=%d\n", SH2_Get_R(sh, 0), SH2_Get_R(sh, 1), SH2_Get_R(sh, 2), SH2_Get_SR(sh) & 1);
	Print_Text(GString, strlen(GString) - 1, 162, 146, WHITE);
	sprintf(GString, "R3=%.8X R4=%.8X R5=%.8X S=%d\n", SH2_Get_R(sh, 3), SH2_Get_R(sh, 4), SH2_Get_R(sh, 5), (SH2_Get_SR(sh) >> 1) & 1);
	Print_Text(GString, strlen(GString) - 1, 162, 154, WHITE);
	sprintf(GString, "R6=%.8X R7=%.8X R8=%.8X Q=%d\n", SH2_Get_R(sh, 6), SH2_Get_R(sh, 7), SH2_Get_R(sh, 8), (SH2_Get_SR(sh) >> 8) & 1);
	Print_Text(GString, strlen(GString) - 1, 162, 162, WHITE);
	sprintf(GString, "R9=%.8X RA=%.8X RB=%.8X M=%d\n", SH2_Get_R(sh, 9), SH2_Get_R(sh, 0xA), SH2_Get_R(sh, 0xB), (SH2_Get_SR(sh) >> 9) & 1);
	Print_Text(GString, strlen(GString) - 1, 162, 170, WHITE);
	sprintf(GString, "RC=%.8X RD=%.8X RE=%.8X I=%.1X\n", SH2_Get_R(sh, 0xC), SH2_Get_R(sh, 0xD), SH2_Get_R(sh, 0xE), (SH2_Get_SR(sh) >> 4) & 0xF);
	Print_Text(GString, strlen(GString) - 1, 162, 178, WHITE);
	sprintf(GString, "RF=%.8X PC=%.8X SR=%.4X St=%.4X\n", SH2_Get_R(sh, 0xF), SH2_Get_PC(sh), SH2_Get_SR(sh), sh->Status & 0xFFFF);
	Print_Text(GString, strlen(GString) - 1, 162, 186, WHITE);
	sprintf(GString, "GBR=%.8X VBR=%.8X PR=%.8X\n", SH2_Get_GBR(sh), SH2_Get_VBR(sh), SH2_Get_PR(sh));
	Print_Text(GString, strlen(GString) - 1, 162, 194, WHITE);
	sprintf(GString, "MACH=%.8X MACL=%.8X IL=%.2X IV=%.2X\n", SH2_Get_MACH(sh), SH2_Get_MACL(sh), sh->INT.Prio, sh->INT.Vect);
	Print_Text(GString, strlen(GString) - 1, 162, 202, WHITE);
}


void Refresh_VDP_State(void)
{
	int tmp;

	sprintf(Str_Tmp,"**** VDP STATUS ****");
	Print_Text(Str_Tmp, 20, 200, 1, GREEN);

	sprintf(GString, "Setting register: 1=%.2X 2=%.2X 3=%.2X 4=%.2X", VDP_Reg.Set1, VDP_Reg.Set2, VDP_Reg.Set3, VDP_Reg.Set4);
	Print_Text(GString, strlen(GString), 162, 14, WHITE);
	sprintf(GString, "Pattern Adr: ScrA=%.2X ScrB=%.2X Win=%.2X", VDP_Reg.Pat_ScrA_Adr, VDP_Reg.Pat_ScrB_Adr, VDP_Reg.Pat_Win_Adr);
	Print_Text(GString, strlen(GString), 162, 22, WHITE);
	sprintf(GString, "Sprite Attribut Adr: Low=%.2X High=%.2X", VDP_Reg.Spr_Att_Adr, VDP_Reg.Reg6);
	Print_Text(GString, strlen(GString), 162, 30, WHITE);
	sprintf(GString, "H Scroll Adr: Low=%.2X High=%.2X",VDP_Reg.H_Scr_Adr, VDP_Reg.Reg14);
	Print_Text(GString, strlen(GString), 162, 38, WHITE);
	sprintf(GString, "H Interrupt=%.2X    Auto Inc=%.2X", VDP_Reg.H_Int, VDP_Reg.Auto_Inc);
	Print_Text(GString, strlen(GString), 162, 46, WHITE);
	sprintf(GString, "BG Color: Low=%.2X Med=%.2X High=%.2X", VDP_Reg.BG_Color, VDP_Reg.Reg8, VDP_Reg.Reg9);
	Print_Text(GString, strlen(GString), 162, 54, WHITE);
	sprintf(GString, "Scroll Size=%.2X    Window Pos: H=%.2X V=%.2X", VDP_Reg.Scr_Size, VDP_Reg.Win_H_Pos, VDP_Reg.Win_V_Pos);
	Print_Text(GString, strlen(GString), 162, 62, WHITE);
	sprintf(GString, "DMA Length: Low=%.2X High=%.2X", VDP_Reg.DMA_Length_L, VDP_Reg.DMA_Length_H);
	Print_Text(GString, strlen(GString), 162, 70, WHITE);
	sprintf(GString, "DMA Source Adr: Low=%.2X Med=%.2X High=%.2X", VDP_Reg.DMA_Src_Adr_L, VDP_Reg.DMA_Src_Adr_M, VDP_Reg.DMA_Src_Adr_H);
	Print_Text(GString, strlen(GString), 162, 78, WHITE);
	tmp = Read_VDP_Status();
	sprintf(GString, "V Int Happened %d  Sprite overflow %d", (tmp >> 7) & 1, (tmp >> 6) & 1);
	Print_Text(GString, strlen(GString), 162, 86, WHITE);
	sprintf(GString, "Collision Spr  %d  Odd Frame in IM %d", (tmp >> 5) & 1, (tmp >> 4) & 1);
	Print_Text(GString, strlen(GString), 162, 94, WHITE);
	sprintf(GString, "During V Blank %d  During H Blank  %d", (tmp >> 3) & 1, (tmp >> 2) & 1);
	Print_Text(GString, strlen(GString), 162, 102, WHITE);
	sprintf(GString, "DMA Busy %d  PAL Mode %d Line Num %d", (tmp >> 1) & 1, tmp & 1, VDP_Current_Line);
	Print_Text(GString, strlen(GString), 162, 110, WHITE);
	sprintf(GString, "VDP Int =%.2X DMA_Length=%.4X", VDP_Int, DMAT_Length);
	Print_Text(GString, strlen(GString), 162, 118, WHITE);
}


void Refresh_VDP_Pattern(void)
{
	unsigned int i;
	
	sprintf(Str_Tmp,"******** VDP PATTERN ********");
	Print_Text(Str_Tmp, 29, 28, 0, GREEN);

	for(i = 0; i < 24; i++)
	{
		sprintf(GString, "%.4X", (pattern_adr & 0xFFFF) + 0x200 * i);
		Print_Text(GString, strlen(GString), 2, (i << 3) + 11, WHITE);
	}
	
	Cell_8x8_Dump(&VRam[pattern_adr & 0xFFFF], pattern_pal);
}


void Refresh_VDP_Palette(void)
{
	unsigned int i, j, k, l;

	sprintf(Str_Tmp,"******** VDP PALETTE ********");
	Print_Text(Str_Tmp, 29, 180, 0, RED);

	// color
	for(i = 0; i < 16; i++)
	{
		// x
		for(j = 0; j < 8; j++)
		{
			// y
			for(k = 0; k < 8; k++)
			{
				// palette
				for (l = 0; l < 4; l++)
				{
					// color from md color + shadow + highlight + alpha
					MD_Screen[(336 * (10 + k + (8 * l))) + 180 + (i * 8) + j] = Palette[CRam[i + (l * 16)] | ((pattern_pal >> 2) << 12) | 0x4000];
					MD_Screen32[(336 * (10 + k + (8 * l))) + 180 + (i * 8) + j] = Palette32[CRam[i + (l * 16)] | ((pattern_pal >> 2) << 12) | 0x4000];
				}
			}
		}
	}
	for (i = 0; i < 16 * 8; i++)
	{
		MD_Screen[(336 * (9 + ((pattern_pal & 3) * 8))) + 180 + i] = 0xFFFF;
		MD_Screen32[(336 * (9 + ((pattern_pal & 3) * 8))) + 180 + i] = 0xFFFFFF;
		MD_Screen[(336 * (18 + ((pattern_pal & 3) * 8))) + 180 + i] = 0xFFFF;
		MD_Screen32[(336 * (18 + ((pattern_pal & 3) * 8))) + 180 + i] = 0xFFFFFF;
	}
	
	sprintf(Str_Tmp,"******** VDP CONTROL ********");
	Print_Text(Str_Tmp, 29, 180, 60, WHITE);

	sprintf(GString, "Status : %.4X", Read_VDP_Status());
	Print_Text(GString, strlen(GString), 176, 70, WHITE);
	sprintf(GString, "Flag : %.2X       Data : %.8X", Ctrl.Flag, Ctrl.Data);
	Print_Text(GString, strlen(GString), 176, 78, WHITE);
	sprintf(GString, "Write : %.2X      Access : %.2X", Ctrl.Write, Ctrl.Access);
	Print_Text(GString, strlen(GString), 176, 86, WHITE);
	sprintf(GString, "Address : %.4X  DMA_Mode : %.2X", Ctrl.Address, Ctrl.DMA_Mode);
	Print_Text(GString, strlen(GString), 176, 94, WHITE);
	sprintf(GString, "DMA adr: %.8X  DMA len: %.4X", VDP_Reg.DMA_Address, VDP_Reg.DMA_Length);
	Print_Text(GString, strlen(GString), 176, 102, WHITE);
	sprintf(GString, "DMA : %.2X", Ctrl.DMA);
	Print_Text(GString, strlen(GString), 176, 110, WHITE);

	sprintf(GString, "Sprite Liste :");
	Print_Text(GString, strlen(GString), 176, 126, WHITE);

	for(i = 0; i < 10; i++)
	{
		sprintf(GString, "%d %d %d %d %d", Sprite_Struct[i].Pos_X , Sprite_Struct[i].Pos_Y, Sprite_Struct[i].Size_X, Sprite_Struct[i].Size_Y, Sprite_Struct[i].Num_Tile);
		Print_Text(GString, strlen(GString), 176, 134 + (i * 8), WHITE);
	}
}


void Refresh_SegaCD_State(void)
{
	sprintf(Str_Tmp,"** SEGACD STATUS **");
	Print_Text(Str_Tmp, 20, 200, 1, GREEN);

	sprintf(GString, "GE00=%.4X GE02=%.4X CD00=%.4X CD02=%.4X", M68K_RW(0xA12000), M68K_RW(0xA12002), S68K_RW(0xFF8000), S68K_RW(0xFF8002));
	Print_Text(GString, strlen(GString), 162, 14, WHITE);
	sprintf(GString, "GE04=%.4X GE06=%.4X CD04=%.4X CD06=%.4X", M68K_RW(0xA12004), M68K_RW(0xA12006), S68K_RW(0xFF8004), 0x0000);
	Print_Text(GString, strlen(GString), 162, 22, WHITE);
	sprintf(GString, "GE0A=%.4X GE0C=%.4X CD0A=%.4X CD0C=%.4X", M68K_RW(0xA1200A), M68K_RW(0xA1200C), S68K_RW(0xFF800A), S68K_RW(0xFF800C));
	Print_Text(GString, strlen(GString), 162, 30, WHITE);
	sprintf(GString, "GD0E=%.4X", S68K_RW(0xFF800E));
	Print_Text(GString, strlen(GString), 162, 38, WHITE);
	sprintf(GString, "GD10=%.4X GD12=%.4X GD14=%.4X GD16=%.4X", S68K_RW(0xFF8010), S68K_RW(0xFF8012), S68K_RW(0xFF8014), S68K_RW(0xFF8016));
	Print_Text(GString, strlen(GString), 162, 46, WHITE);
	sprintf(GString, "GD18=%.4X GD1A=%.4X GD1C=%.4X GD1E=%.4X", S68K_RW(0xFF8018), S68K_RW(0xFF801A), S68K_RW(0xFF801C), S68K_RW(0xFF801E));
	Print_Text(GString, strlen(GString), 162, 54, WHITE);
	sprintf(GString, "GD20=%.4X GD22=%.4X GD24=%.4X GD26=%.4X", S68K_RW(0xFF8020), S68K_RW(0xFF8022), S68K_RW(0xFF8024), S68K_RW(0xFF8026));
	Print_Text(GString, strlen(GString), 162, 62, WHITE);
	sprintf(GString, "GD28=%.4X GD2A=%.4X GD2C=%.4X GD2E=%.4X", S68K_RW(0xFF8028), S68K_RW(0xFF802A), S68K_RW(0xFF802C), S68K_RW(0xFF802E));
	Print_Text(GString, strlen(GString), 162, 70, WHITE);
	sprintf(GString, "CD30=%.4X CD32=%.4X CD34=%.4X CD36=%.4X", S68K_RW(0xFF8030), S68K_RW(0xFF8032), S68K_RW(0xFF8034), S68K_RW(0xFF8036));
	Print_Text(GString, strlen(GString), 162, 78, WHITE);
	sprintf(GString, "CD38=%.4X CD3A=%.4X CD3E=%.4X CD40=%.4X", S68K_RW(0xFF8038), S68K_RW(0xFF803A), S68K_RW(0xFF803E), S68K_RW(0xFF8040));
	Print_Text(GString, strlen(GString), 162, 86, WHITE);
	sprintf(GString, "CD42=%.4X CD44=%.4X CD48=%.4X CD4A=%.4X", S68K_RW(0xFF8042), S68K_RW(0xFF8044), S68K_RW(0xFF8048), S68K_RW(0xFF804A));
	Print_Text(GString, strlen(GString), 162, 94, WHITE);
	sprintf(GString, "CD4C=%.4X CD4E=%.4X CD50=%.4X CD52=%.4X", S68K_RW(0xFF804C), S68K_RW(0xFF804E), S68K_RW(0xFF8050), S68K_RW(0xFF8052));
	Print_Text(GString, strlen(GString), 162, 102, WHITE);
	sprintf(GString, "CD58=%.4X CD5A=%.4X CD5C=%.4X CD5E=%.4X", S68K_RW(0xFF8058), S68K_RW(0xFF805A), S68K_RW(0xFF805C), S68K_RW(0xFF805E));
	Print_Text(GString, strlen(GString), 162, 110, WHITE);
	sprintf(GString, "CD60=%.4X CD62=%.4X CD64=%.4X CD66=%.4X", S68K_RW(0xFF8060), S68K_RW(0xFF8062), S68K_RW(0xFF8064), S68K_RW(0xFF8066));
	Print_Text(GString, strlen(GString), 162, 118, WHITE);
}


void Refresh_32X_State(void)
{
	sprintf(Str_Tmp,"*** 32X STATUS ***");
	Print_Text(Str_Tmp, 20, 200, 1, GREEN);

	sprintf(GString, "M000=%.4X S000=%.4X M004=%.4X M006=%.4X", SH2_Read_Word(&M_SH2, 0x4000), SH2_Read_Word(&S_SH2, 0x4000), SH2_Read_Word(&M_SH2, 0x4004), SH2_Read_Word(&M_SH2, 0x4006));
	Print_Text(GString, strlen(GString), 162, 14, WHITE);
	sprintf(GString, "M008=%.4X M00A=%.4X M00C=%.4X M00E=%.4X", SH2_Read_Word(&M_SH2, 0x4008), SH2_Read_Word(&M_SH2, 0x400A), SH2_Read_Word(&M_SH2, 0x400C), SH2_Read_Word(&M_SH2, 0x400E));
	Print_Text(GString, strlen(GString), 162, 22, WHITE);
	sprintf(GString, "M010=%.4X M012=%.4X M014=%.4X M016=%.4X", SH2_Read_Word(&M_SH2, 0x4010), SH2_Read_Word(&M_SH2, 0x4012), SH2_Read_Word(&M_SH2, 0x4014), SH2_Read_Word(&M_SH2, 0x4016));
	Print_Text(GString, strlen(GString), 162, 30, WHITE);
	sprintf(GString, "M020=%.4X M022=%.4X M024=%.4X M026=%.4X", SH2_Read_Word(&M_SH2, 0x4020), SH2_Read_Word(&M_SH2, 0x4022), SH2_Read_Word(&M_SH2, 0x4024), SH2_Read_Word(&M_SH2, 0x4026));
	Print_Text(GString, strlen(GString), 162, 38, WHITE);
	sprintf(GString, "M028=%.4X M02A=%.4X M02C=%.4X M02E=%.4X", SH2_Read_Word(&M_SH2, 0x4028), SH2_Read_Word(&M_SH2, 0x402A), SH2_Read_Word(&M_SH2, 0x402C), SH2_Read_Word(&M_SH2, 0x402E));
	Print_Text(GString, strlen(GString), 162, 46, WHITE);
	sprintf(GString, "M030=%.4X M032=%.4X M034=%.4X M036=%.4X", SH2_Read_Word(&M_SH2, 0x4030), SH2_Read_Word(&M_SH2, 0x4032), SH2_Read_Word(&M_SH2, 0x4034), SH2_Read_Word(&M_SH2, 0x4036));
	Print_Text(GString, strlen(GString), 162, 54, WHITE);
	sprintf(GString, "M100=%.4X M102=%.4X M104=%.4X M106=%.4X", SH2_Read_Word(&M_SH2, 0x4100), SH2_Read_Word(&M_SH2, 0x4102), SH2_Read_Word(&M_SH2, 0x4104), SH2_Read_Word(&M_SH2, 0x4106));
	Print_Text(GString, strlen(GString), 162, 62, WHITE);
	sprintf(GString, "M108=%.4X M10A=%.4X M10C=%.4X M10E=%.4X", SH2_Read_Word(&M_SH2, 0x4108), SH2_Read_Word(&M_SH2, 0x410A), SH2_Read_Word(&M_SH2, 0x410C), SH2_Read_Word(&M_SH2, 0x410E));
	Print_Text(GString, strlen(GString), 162, 70, WHITE);
}


void Refresh_CDC_State(void)
{
	sprintf(Str_Tmp,"** CDC STATUS **");
	Print_Text(Str_Tmp, 16, 200, 1, GREEN);

	sprintf(GString, "COMIN=%.2X IFSTAT=%.2X DBC=%.4X", CDC.COMIN, CDC.IFSTAT, CDC.DBC);
	Print_Text(GString, strlen(GString), 162, 14, WHITE);
	sprintf(GString, "HEAD=%.8X PT=%.4X WA=%.4X", CDC.HEAD, CDC.PT, CDC.WA);
	Print_Text(GString, strlen(GString), 162, 22, WHITE);
	sprintf(GString, "STAT=%.8X CTRL=%.8X", CDC.STAT, CDC.CTRL);
	Print_Text(GString, strlen(GString), 162, 30, WHITE);
	sprintf(GString, "DAC=%.4X IFCTRL=%.2X", CDC.DAC, CDC.IFCTRL);
	Print_Text(GString, strlen(GString), 162, 38, WHITE);
}


void Refresh_Word_Ram_Pattern(void)
{
	unsigned int i, j, k, l;
	
	sprintf(Str_Tmp,"****** WORD RAM PATTERN ******");
	Print_Text(Str_Tmp, 29, 28, 0, GREEN);

	for(i = 0; i < 24; i++)
	{
		sprintf(GString, "%.4X", (cd_pattern_adr & 0x3FFFF) + 0x200 * i);
		Print_Text(GString, strlen(GString), 2, (i << 3) + 11, WHITE);
	}
	if (Rot_Comp.Stamp_Size & 2) Cell_32x32_Dump(&Ram_Word_2M[cd_pattern_adr & 0x3FFFF], pattern_pal);
	else Cell_16x16_Dump(&Ram_Word_2M[cd_pattern_adr & 0x3FFFF], pattern_pal);

	sprintf(Str_Tmp,"******** VDP PALETTE ********");
	Print_Text(Str_Tmp, 29, 180, 0, RED);

	// color
	for(i = 0; i < 16; i++)
	{
		// x
		for(j = 0; j < 8; j++)
		{
			// y
			for(k = 0; k < 8; k++)
			{
				// palette
				for (l = 0; l < 16; l++)
				{
					// color from md color + shadow + highlight + alpha
					MD_Screen[(336 * (10 + k + (8 * l))) + 180 + (i * 8) + j] = Palette[CRam[i + ((l&3) * 16)] | ((l >> 2) << 12) | 0x4000];
					MD_Screen32[(336 * (10 + k + (8 * l))) + 180 + (i * 8) + j] = Palette32[CRam[i + ((l&3) * 16)] | ((l >> 2) << 12) | 0x4000];
				}
			}
		}
	}
	for (i = 0; i < 16 * 8; i++)
	{
		MD_Screen[(336 * (9 + (pattern_pal * 8))) + 180 + i] = 0xFFFF;
		MD_Screen32[(336 * (9 + (pattern_pal * 8))) + 180 + i] = 0xFFFFFF;
		MD_Screen[(336 * (18 + (pattern_pal * 8))) + 180 + i] = 0xFFFF;
		MD_Screen32[(336 * (18 + (pattern_pal * 8))) + 180 + i] = 0xFFFFFF;
	}
}


void Update_Debug_Screen(void)
{
	memset(MD_Screen, 0, 336 * 240 * 2);
	memset(MD_Screen32, 0, 336 * 240 * 4);
	
	if (Debug & 0x100) Do_VDP_Refresh();
	else switch(Debug)
	{
		default:
		case 1:		// Main 68000
			Refresh_M68k_Mem();
			Refresh_M68k_Inst();
			Refresh_M68k_State();
			Refresh_VDP_State();
			break;

		case 2:		// Z80
			Refresh_Z80_Mem();
			Refresh_Z80_Inst();
			Refresh_Z80_State();
			break;

		case 3:		// Genesis VDP
			Refresh_VDP_Pattern();
			Refresh_VDP_Palette();
			break;

		case 4:		// Sub 68000 Reg
			Refresh_S68k_Mem();
			Refresh_S68k_Inst();
			Refresh_S68k_State();
			Refresh_SegaCD_State();
			break;

		case 5:		// Sub 68000 CDC
			Refresh_S68k_Mem();
			Refresh_S68k_Inst();
			Refresh_S68k_State();
			Refresh_CDC_State();
			break;

		case 6:		// Vector chip pattern
			Refresh_Word_Ram_Pattern();
			break;

		case 7:		// Main SH2
			Refresh_SH2_Mem();
			Refresh_SH2_Inst(0);
			Refresh_SH2_State(0);
			Refresh_32X_State();
			break;

		case 8:		// Sub SH2
			Refresh_SH2_Mem();
			Refresh_SH2_Inst(1);
			Refresh_SH2_State(1);
			Refresh_32X_State();
			break;

		case 9:		// 32X VDP
			_32X_VDP_Draw(Current_32X_FB);
			break;
	}

	Sleep(10);
}




#endif //GENS_DEBUG