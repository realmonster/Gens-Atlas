#include "main.h"

typedef void (*object_render)(int arg, int flags, int x, int y);

struct object_desc
{
	int invisible, mapping, base_tile, use_arg;
	object_render render;
} objects[0x100] = {
	0, 0x1A99A, 0xA6BC, 0, 0, //  0 Ring
	0, 0x1DBA2,  0x4C4, 1, 0, //  1 Monitor
	1,       0,      0, 0, 0, //  2 Collision plane switcher
	0,       0,      0, 0, 0, //  3 Large tree from AIZ 1
	0,       0,      0, 0, 0, //  4 Crumbling platforms from AIZ and ICZ
	0,       0,      0, 0, 0, //  5 Rock from AIZ, LRZ and EMZ
	0,       0,      0, 0, 0, //  6 Vine which slides down a rope from AIZ 1
	0,       0,      0, 0, 0, //  7 Spring
	0,       0,      0, 0, 0, //  8 Spikes
	0,       0,      0, 0, 0, //  9 Tree bark from AIZ 1
	0,       0,      0, 0, 0, //  A Rope peg from AIZ 1
	0,       0,      0, 0, 0, //  B Ring (unused slot)
	0,       0,      0, 0, 0, //  C Swinging vine from AIZ
	0,       0,      0, 0, 0, //  D Breakable wall
	0,       0,      0, 0, 0, //  E Ridge
	0,       0,      0, 0, 0, //  F Collapsing platform/bridge/ledge
	0,       0,      0, 0, 0, // 10
	0,       0,      0, 0, 0, // 11
	0,       0,      0, 0, 0, // 12
	0,       0,      0, 0, 0, // 13
	0,       0,      0, 0, 0, // 14
	0,       0,      0, 0, 0, // 15
	0,       0,      0, 0, 0, // 16
	0,       0,      0, 0, 0, // 17
	0,       0,      0, 0, 0, // 18
	0,       0,      0, 0, 0, // 19
	0,       0,      0, 0, 0, // 1A
	0,       0,      0, 0, 0, // 1B
	0,       0,      0, 0, 0, // 1C
	0,       0,      0, 0, 0, // 1D
	0,       0,      0, 0, 0, // 1E
	0,       0,      0, 0, 0, // 1F
	0,       0,      0, 0, 0, // 20
	0,       0,      0, 0, 0, // 21
	0,       0,      0, 0, 0, // 22
	0,       0,      0, 0, 0, // 23
	0,       0,      0, 0, 0, // 24
	0,       0,      0, 0, 0, // 25
	0,       0,      0, 0, 0, // 26
	0,       0,      0, 0, 0, // 27
	0,       0,      0, 0, 0, // 28
	0,       0,      0, 0, 0, // 29
	0,       0,      0, 0, 0, // 2A
	0,       0,      0, 0, 0, // 2B
	0,       0,      0, 0, 0, // 2C
	0,       0,      0, 0, 0, // 2D
	0,       0,      0, 0, 0, // 2E
	0,       0,      0, 0, 0, // 2F
	0,       0,      0, 0, 0, // 30
	0,       0,      0, 0, 0, // 31
	0,       0,      0, 0, 0, // 32
	0,       0,      0, 0, 0, // 33
	0,       0,      0, 0, 0, // 34
	0,       0,      0, 0, 0, // 35
	0,       0,      0, 0, 0, // 36
	0,       0,      0, 0, 0, // 37
	0,       0,      0, 0, 0, // 38
	0,       0,      0, 0, 0, // 39
	0,       0,      0, 0, 0, // 3A
	0,       0,      0, 0, 0, // 3B
	0,       0,      0, 0, 0, // 3C
	0,       0,      0, 0, 0, // 3D
	0,       0,      0, 0, 0, // 3E
	0,       0,      0, 0, 0, // 3F
	0,       0,      0, 0, 0, // 40
	0,       0,      0, 0, 0, // 41
	0,       0,      0, 0, 0, // 42
	0,       0,      0, 0, 0, // 43
	0,       0,      0, 0, 0, // 44
	0,       0,      0, 0, 0, // 45
	0,       0,      0, 0, 0, // 46
	0,       0,      0, 0, 0, // 47
	0,       0,      0, 0, 0, // 48
	0,       0,      0, 0, 0, // 49
	0,       0,      0, 0, 0, // 4A
	0,       0,      0, 0, 0, // 4B
	0,       0,      0, 0, 0, // 4C
	0,       0,      0, 0, 0, // 4D
	0,       0,      0, 0, 0, // 4E
	0,       0,      0, 0, 0, // 4F
};

void DrawObject(int id, int arg, int flags, int x, int y)
{
	object_desc &obj = objects[id];
	if (obj.invisible)
		return;
	if (obj.mapping)
	{
		int frame = 0;
		if (obj.use_arg)
			frame = arg+1;
		DrawFrame(x,y,
				obj.mapping, // offset
				frame, // frame
				0, // flags
				obj.base_tile); // base
	}
	else
	{
		DrawFrame(x,y,
				objects[0].mapping, // offset
				0, // frame
				0, // flags
				objects[0].base_tile); // base
	}
}
