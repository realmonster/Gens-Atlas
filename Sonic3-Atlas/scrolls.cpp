#include "scrolls.h"

void ParallaxScroll(int Heights, int Values)
{
	int j = Values;
	int l = 0;
	for (int i=Heights; i<Heights+0x50; i+=2)
	{
		int v = *(WORD*)&ROM[i];
		if (v & 0x8000)
		{
			// parallax
			v -= 0x8000;
			for (int k=0; k<v; ++k)
			{
				HSCROLL[l]=*(WORD*)&RAM[j];
				j+=2;
				++l;
				if (l >= 0x1000)
					break;
			}
		}
		else
		{
			for (int k=0; k<v; ++k)
			{
				HSCROLL[l]=*(WORD*)&RAM[j];
				++l;
				if (l >= 0x1000)
					break;
			}
			j+=2;
		}
		if (l >= 0x1000)
			break;
	}
}

void ParallaxSize(int Heights)
{
	int l = 0;
	for (int i=Heights; i<Heights+0x50; i+=2)
	{
		int v = *(WORD*)&ROM[i];
		if (v & 0x8000)
		{
			// parallax
			v -= 0x8000;
		}
		int val = (v > 0x1000)?*(WORD*)&RAM[0x8002]:4;
		for (int k=0; k<v; ++k)
		{
			HSIZE[l] = val;
			++l;
			if (l >= 0x1000)
				break;
		}
		if (l >= 0x1000)
			break;
	}
}

void AllSize(int Width)
{
	for (int i=0; i<0x20*128; ++i)
	{
		HSIZE[i] = Width;
	}
}

void AngelIslandZoneScroll1()
{
	ParallaxScroll(0x23B258,0xA808);
	ParallaxSize(0x23B258);
}

void AngelIslandZoneScroll2()
{
	ParallaxScroll(0x23BC06,0xA9C0);
	ParallaxSize(0x23BC06);
}

void HydrocityScroll1()
{
	ParallaxScroll(0x23C590,0xA800);
	ParallaxSize(0x23C590);
}

void HydrocityScroll2()
{
	ParallaxScroll(0x23C7D2,0xA800);
	AllSize(4);
}

void MarbleGardenZoneScroll1()
{
	ParallaxScroll(0x23C9F8,0xA800);
	ParallaxSize(0x23C9F8);
}

void MarbleGardenZoneScroll2()
{
	//EE94, EE95, EEC8, F664
	//int sign = *(WORD*)&RAM[0xEED2]==8?-1:1;
	ParallaxScroll(0x23D498,0xA808);
	ParallaxSize(0x23D498);
}

void CarnavalNightZoneScroll1()
{
	if (*(WORD*)&RAM[0xEEC2] == 4)
	{
		for (int l=0; l<0x20*128; ++l)
		{
			HSCROLL[l]=*(WORD*)&RAM[0xEE8C];
		}
		AllSize(*(WORD*)&RAM[0x8002]);
	}
	else
	{
		ParallaxScroll(0x520FE,0xA800);
		ParallaxSize(0x520FE);
	}
}

void FlyingBatteryZoneScroll1()
{
	if (*(WORD*)&RAM[0xEED6])
	{
		// sky
		ParallaxScroll(0x52D6E,0xA800);
	}
	else
	{
		// inner
		ParallaxScroll(0x52D28,0xA800);
		ParallaxSize(0x52D28);
	}
}

void IceCapZoneScroll1()
{
	if (*(WORD*)&RAM[0xEEC2]==0x4)
	{
		// third part
		if (*(WORD*)&RAM[0xEEE8])
		{
			ParallaxScroll(0x23E246,0xA800);
			ParallaxSize(0x23E246);
		}
		else
		{
			ParallaxScroll(0x23E23E,0xA800);
			ParallaxSize(0x23E23E);
		}
	}
	else if (*(WORD*)&RAM[0xEEC2]!=0x10)
	{
		// on top
		int j = 0xA800;
		int l = 0;
		for (int i=0x23DEFA; i<0x23DFFA; i+=2)
		{
			int v = *(WORD*)&ROM[i];
			if (v & 0x8000)
			{
				// parallax
				v -= 0x8000;
				for (int k=0; k<v; ++k)
				{
					PAL[0x80+l]=(*(WORD*)&RAM[j]);
					j+=2;
					++l;
					if (l >= 0x1000)
						break;
				}
			}
			else
			{
				for (int k=0; k<v; ++k)
				{
					PAL[0x80+l]=(*(WORD*)&RAM[j]);
					++l;
					if (l >= 0x1000)
						break;
				}
				j+=2;
			}
			if (l >= 0x1000)
				break;
		}
	}
	else
	{
		// inside
		for (int l=0; l<0x20*128; ++l)
		{
			HSCROLL[l]=*(WORD*)&RAM[0xEE8C];
			HSIZE[l]=*(WORD*)&RAM[0x8002];
		}
	}
}

void LaunchBaseZoneScroll1()
{
	ParallaxScroll(0x5433E,0xA808);
	AllSize(4);
	for (int i=0; i<0xD0; ++i)
		HSIZE[i] = 0xC;
}

void LaunchBaseZoneScroll2()
{
	if (*(WORD*)&RAM[0xEEC2] != 0xC)
	{
		ParallaxScroll(0x23EEE0,0xA800);
		ParallaxSize(0x23EEE0);
	}
	else // end of level
	{
		ParallaxScroll(0x23EF04,0xA800);
		ParallaxSize(0x23EF04);
	}
}

void MushroomHillZoneScroll1()
{
	for (int l=0; l<0x20*128; ++l)
	{
		HSCROLL[l]=*(WORD*)&RAM[0xEE8C];
	}
	AllSize(*(WORD*)&RAM[0x8002]);
}

void SandpolisZoneScroll1()
{
	ParallaxScroll(0x560DC,0xA800);
	AllSize(4);
}

void BonusLevelScroll1()
{
	for (int l=0; l<0x20*128; ++l)
	{
		HSCROLL[l]=*(WORD*)&RAM[0xEE8C];
	}
	AllSize(*(WORD*)&RAM[0x8002]);
}

void BonusLevelScroll2()
{
	ParallaxScroll(0x599BE,0xA800);
	AllSize(4);
}

void UpdatePalAndScroll()
{
	memcpy(PAL,CRAM.data,0x80*2);
	switch(*(WORD*)&RAM[0xFE10])
	{
		case 0x000:
			AngelIslandZoneScroll1();
			break;
		case 0x001:
			AngelIslandZoneScroll2();
			break;
		case 0x100:
			HydrocityScroll1();
			break;
		case 0x101:
			HydrocityScroll2();
			break;
		case 0x200:
			MarbleGardenZoneScroll1();
			break;
		case 0x201:
			MarbleGardenZoneScroll2();
			break;
		case 0x300:
		case 0x301:
			CarnavalNightZoneScroll1();
			break;
		case 0x400:
		case 0x401:
			FlyingBatteryZoneScroll1();
			break;
		case 0x500:
		case 0x501:
			IceCapZoneScroll1();
			break;
		case 0x600:
			LaunchBaseZoneScroll1();
			break;
		case 0x601:
			LaunchBaseZoneScroll2();
			break;
		case 0x700:
			MushroomHillZoneScroll1();
			break;
		case 0x701:
			MushroomHillZoneScroll1();
			break;
		case 0x800:
			SandpolisZoneScroll1();
			break;
		case 0x1400:
			BonusLevelScroll1();
			break;
		case 0x1500:
			BonusLevelScroll2();
			break;
		default:
			memset(PAL+0x80,0x1000,0);
			break;
	}
}
