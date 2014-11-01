#include "main.h"

inline int DESC_ARG(int desc)
{
	return ROM[(desc+5)^1];
}

inline int DESC_ID(int desc)
{
	return ROM[(desc+4)^1];
}

inline int DESC_X(int desc)
{
	return *(WORD*)&ROM[desc];
}

inline int DESC_Y(int desc)
{
	return (*(WORD*)&ROM[desc+2])&0xFFF;
}

inline int DESC_HV(int desc)
{
	return ((*(WORD*)&ROM[desc+2])>>13)&3;
}

inline int camerax()
{
	return *(WORD*)&RAM[0xEE78];
}

inline int cameray()
{
	return *(WORD*)&RAM[0xEE7C];
}

inline int zone()
{
	return RAM[0xFE10^1];
}

inline int zone_and_act()
{
	return *(WORD*)&RAM[0xFE10];
}

typedef void (*object_render)(int desc, int status);

static void DrawDummy(int desc, int mapping, int frame, int base)
{
	int x = DESC_X(desc) - camerax();
	int y = DESC_Y(desc) - cameray();
	int flags = DESC_HV(desc);
	DrawFrame(x,y,mapping,frame,flags,base);
}

static void static_ring(int desc, int status)
{
	DrawDummy(desc, 0x1A99A, 0, 0xA6BC); // static ring
}

static void obj_monitor(int desc, int status)
{
	if (status & 1) // broken
		DrawDummy(desc, 0x1DBA2, 0xB, 0x4C4);
	else
	{
		int frame = *(WORD*)&RAM[0xFE04];
		frame >>= 1;
		if ((frame % 6) == 0)
			frame = 0;
		else if ((frame % 6) == 3)
			frame = 1;
		else
			frame = DESC_ARG(desc)+1;
		DrawDummy(desc, 0x1DBA2, frame, 0x4C4);
	}
}

// 5
static void obj_rock(int desc, int status)
{
	if (!(status & 0x80))
	{
		int map = 0x21DCDC;
		int base = 0x2333;
		int frame = (DESC_ARG(desc)>>4)&7;
		if (zone_and_act() == 1)
		{
			map = 0x21DD64;
			base = 0x42E9;
		}
		else if (zone_and_act() == 0x1200)
		{
			map = 0x21DDEC;
			base = 0xE300;
			frame = 0;
		}
		else if (zone_and_act() == 0x900)
		{
			map = 0x203D8;
			base = 0x40D3;
			frame += 4;
		}
		else if (zone_and_act() == 0x901)
		{
			map = 0x2047A;
			base = 0x640D;
			frame += 4;
		}
		int x = DESC_X(desc) - camerax() - (status&0x7F);
		int y = DESC_Y(desc) - cameray();
		int flags = DESC_HV(desc);
		DrawFrame(x,y,map,frame,flags,base);
	}
}

// 7
static void obj_spring(int desc, int status)
{
	if (status & 0x80) // loaded
		return;
	int arg = DESC_ARG(desc);
	int comp = *(WORD*)&RAM[0xFFE8]; // competition
	int map;
	int base = 0x4B4;
	if (arg & 2)
	{
		// yellow spring
		if (comp)
			map = 0x23912;
		else
			map = 0x23772;
	}
	else
	{
		// red spring
		if (comp)
			map = 0x23904;
		else
			map = 0x2375C;
	}
	int frame;
	int flags = DESC_HV(desc);
	switch(arg>>4)
	{
	case 0:
		base = 0x4A4;
		frame = 0;
		break;
	case 1:
		base = 0x4B4;
		frame = 3;
		break;
	case 2:
		base = 0x4A4;
		frame = 6;
		break;
	case 3:
		base = 0x43A;
		if (zone() == 2 || zone() == 7)
			base = 0x478;
		frame = 7;
		break;
	case 4:
		base = 0x43A;
		if (zone() == 2 || zone() == 7)
			base = 0x478;
		frame = 10;
		break;
	}
	int x = DESC_X(desc) - camerax();
	int y = DESC_Y(desc) - cameray();
	DrawFrame(x,y,map,frame,flags,base);
}

// 8
static void obj_spikes(int desc, int status)
{
	if (status & 0x80) // loaded
		return;
	int arg = DESC_ARG(desc);
	int comp = *(WORD*)&RAM[0xFFE8]; // competition
	int map = 0x24456;
	int base = 0x49C;
	int frame = (arg >> 4);
	if (zone() == 4)
		base = 0x200;
	if ((arg >> 4)>=4)
		base = 0x494;
	int x = DESC_X(desc) - camerax();
	int y = DESC_Y(desc) - cameray();
	if ((arg&3) == 1 || (arg&3) == 2)
	{
		// moving
		int frame = (*(WORD*)&RAM[0xFE04])&0x7F;
		int delta;
		if (frame < 0x40)
		{
			if (frame < 4)
				delta = 8*frame;
			else
				delta = 0x20;
		}
		else
		{
			if (frame < 0x44)
				delta = 0x20-(8*(frame-0x40));
			else
				delta = 0;
		}
		if ((arg&3) == 1)
			y += delta;
		else
			x += delta;
	}
	DrawFrame(x,y,map,frame,DESC_HV(desc),base);
}

// 0x51
static void obj_floatingplatform(int desc, int status)
{
	int map = 0x256A2;
	int base = 0x43F7;
	if (zone_and_act() == 1)
		base = 0x4440;
	else if (zone() == 1)
	{
		map = 0x25688;
		base = 0x441D;
	}
	else if (zone() == 2)
	{
		map = 0x25654;
		base = 0x4001;
	}
	int x = DESC_X(desc) - camerax();
	int y = DESC_Y(desc) - cameray();
	int arg = DESC_ARG(desc)&0xF;
	int hv = DESC_HV(desc);
	if (arg == 1)
	{
		if (hv & 1)
			x += 0x40-RAM[0xFE78^1];
		else
			x += RAM[0xFE78^1];
	}
	else if (arg == 2 || arg == 5)
	{
		if (hv & 1)
			x += 0x80-RAM[0xFE8C^1];
		else
			x += RAM[0xFE8C^1];
	}
	else if (arg == 3)
	{
		if (hv & 1)
			y -= 0x40-RAM[0xFE78^1];
		else
			y -= RAM[0xFE78^1];
	}
	else if (arg == 4)
	{
		if (hv & 1)
			y -= 0x80-RAM[0xFE8C^1];
		else
			y -= RAM[0xFE8C^1];
	}
	else if (arg == 6)
	{
		if (hv & 1)
			x += 0x80-(0x80-RAM[0xFE8C^1]);
		else
			x += (0x80-RAM[0xFE8C^1]);
	}
	else if (arg == 7)
	{
		if (status & 0x80) // loaded
			return;
	}
	if (arg == 5 || arg == 6)
	{
		if (hv & 1)
			y -= 0x40-(RAM[0xFE8C^1]>>1);
		else
			y -= (RAM[0xFE8C^1]>>1);
	}
	DrawFrame(x,y,map,0,hv,base);
}

static object_render objects[0x100] = {
	static_ring, //  0 Ring
	obj_monitor, //  1 Monitor
	0,           //  2 Collision plane switcher
	0,           //  3 Large tree from AIZ 1
	0,           //  4 Crumbling platforms from AIZ and ICZ
	obj_rock,    //  5 Rock from AIZ, LRZ and EMZ
	0,           //  6 Vine which slides down a rope from AIZ 1
	obj_spring,  //  7 Spring
	obj_spikes,  //  8 Spikes
	0,           //  9 Tree bark from AIZ 1
	0,           //  A Rope peg from AIZ 1
	0,           //  B Ring (unused slot)
	0,           //  C Swinging vine from AIZ
	0,           //  D Breakable wall
	0,           //  E Ridge
	0,           //  F Collapsing platform/bridge/ledge
	0,           // 10
	0,           // 11
	0,           // 12
	0,           // 13
	0,           // 14
	0,           // 15
	0,           // 16
	0,           // 17
	0,           // 18
	0,           // 19
	0,           // 1A
	0,           // 1B
	0,           // 1C
	0,           // 1D
	0,           // 1E
	0,           // 1F
	0,           // 20
	0,           // 21
	0,           // 22
	0,           // 23
	0,           // 24
	0,           // 25
	0,           // 26
	0,           // 27
	0,           // 28
	0,           // 29
	0,           // 2A
	0,           // 2B
	0,           // 2C
	0,           // 2D
	0,           // 2E
	0,           // 2F
	0,           // 30
	0,           // 31
	0,           // 32
	0,           // 33
	0,           // 34
	0,           // 35
	0,           // 36
	0,           // 37
	0,           // 38
	0,           // 39
	0,           // 3A
	0,           // 3B
	0,           // 3C
	0,           // 3D
	0,           // 3E
	0,           // 3F
	0,           // 40
	0,           // 41
	0,           // 42
	0,           // 43
	0,           // 44
	0,           // 45
	0,           // 46
	0,           // 47
	0,           // 48
	0,           // 49
	0,           // 4A
	0,           // 4B
	0,           // 4C
	0,           // 4D
	0,           // 4E
	0,           // 4F
	0,           // 50
	obj_floatingplatform, // 51
	0,           // 52
	0,           // 53
	0,           // 54
	0,           // 55
	0,           // 56
	0,           // 57
	0,           // 58
	0,           // 59
	0,           // 5A
	0,           // 5B
	0,           // 5C
	0,           // 5D
	0,           // 5E
	0,           // 5F
};

// desc = rom address
// word: X
// word: AVH0 YYYY YYYY YYYY (A - y check)
// byte: id
// byte: arg
// status = status byte in status table
void DrawObject(int desc, int status)
{
	int id = DESC_ID(desc);
	if (objects[id])
		objects[id](desc,status);
}

// must be sorted increasing
int invisible[] = {
	0x1CD8A, // Collision plane switch
	0x1CEF2, // Breakable floor debug sprite
	0x1E896, // "Pinball mode" toggle tag
	0x1EC6C, // Invisible barrier
};

#include <algorithm>

bool ObjectInvisible(int code)
{
	//return std::binary_search(invisible,invisible+(sizeof(invisible)/sizeof(invisible[0])),code);
	int l=0, r=sizeof(invisible)/sizeof(invisible[0]);
	while (l < r)
	{
		int x = (l + r)>>1;
		if (invisible[x] < code)
			l = x + 1;
		else
			r = x;
	}
	if (l == sizeof(invisible)/sizeof(invisible[0]))
		return false;
	if (invisible[l] == code)
		return true;
	return false;
}
