#ifndef MAIN_H
#define MAIN_H
#pragma once

#include <gl/glew.h> 
#include <gl/wglew.h>

template <typename T>
struct SafeArray
{
	const T* data;
	int mask;
	const T & operator [] (int n) const
	{
		return data[n&mask];
	}
};

extern SafeArray<BYTE> ROM;
extern SafeArray<BYTE> RAM;
extern SafeArray<BYTE> VRAM;
extern SafeArray<WORD> CRAM;

extern WORD PAL[0x80+0x2000];

extern void DrawFrame(int offset, int frame, int flags, int base, int x, int y);
extern void DrawSprite(int id, int x, int y, int size);

#endif
