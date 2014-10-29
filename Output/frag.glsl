// background shader for Sonic 3 & Knuckles
// by r57shell
 
#version 140
 
precision highp float; // needed only for version 1.30

uniform isamplerBuffer ROM;
uniform isamplerBuffer PAL;
uniform isamplerBuffer VRAM;
uniform isamplerBuffer RAM;

in vec3 in_Position;
out vec4 out_Color;

ivec4 _texelFetch(isamplerBuffer b, int pos)
{
	return texelFetch(b, pos^1);
}

vec4 getPal(int id)
{
	int b = _texelFetch(PAL,id*2).r;
	int gr = _texelFetch(PAL,id*2+1).r;
	int g = (gr >> 4) & 0xE; 
	int r = gr & 0xE;
	int a = 14;
	if ((id & 0xF) == 0)
		a = 0;
	return vec4(r,g,b,a)/14.0;
}

vec4 getTile(int id, int x, int y)
{
	int pccvh = id >> 11;
	int tid = id & 2047;
	int pccv = pccvh >> 1;
	int h  = pccvh & 1;
	int pcc = pccv >> 1;
	int v  = pccv & 1;
	int p = pcc & 4;
	int cc = pcc & 3;
	if (h != 0)
		x = 7 - x;
	if (v != 0)
		y = 7 - y;

	int xx = x >> 1;
	int xy = x & 1;
	int k = _texelFetch(VRAM, tid*32+y*4+xx).r;
	vec4 r;
	if (xy == 0)
		r = getPal((k >> 4) + cc*16);
	else
		r = getPal((k & 15) + cc*16);
	r.a += p;
	return r;
}

vec4 getBlock16(int id, int x, int y)
{
	if ((id & 1024) != 0)
		x = 15 - x;
	if ((id & 2048) != 0)
		y = 15 - y;

	int xx = x >> 3;
	int yy = y >> 3;
	int start = 0x9000 + 8*(id&1023) + xx*2 + yy*4;
	int hw = _texelFetch(RAM,start).r;
	int lw = _texelFetch(RAM,start+1).r;
	xx = x & 7;
	yy = y & 7;
	return getTile(hw*256+lw, xx, yy);
}

vec4 getBlock128(int id, int x, int y)
{
	int xx = x >> 4;
	int yy = y >> 4;
	int start = 128*id + xx*2 + yy*16;
	int hw = _texelFetch(RAM,start).r;
	int lw = _texelFetch(RAM,start+1).r;
	return getBlock16(hw*256+lw, x&15, y&15);
}

vec4 background(int id, int x, int y)
{
	int w = _texelFetch(RAM, 0x8000+id*2+1).r;
	int h = _texelFetch(RAM, 0x8000+id*2+5).r;
	if (y < 0)
		y = (y%(h*128))+(h*128);
	else
		y = y%(h*128);
	if (id == 0)
	{
	if (x < 0)
		x = (x%(w*128))+(w*128);
	else
		x = x%(w*128);
	}

	if (id == 1)
	{
		x += ((_texelFetch(PAL,0x100+y*2).r<<8)|_texelFetch(PAL,0x101+y*2).r);
		w = _texelFetch(PAL,0x2100+y).r;
		//x -= -30;//128+58;
		if (x < 0)
			x = (x%(w*128))+(w*128);
		else
			x = x%(w*128);
		/*if (x < 0)
			x += ((-x)/(w*128)+1)*w*128;
		x -= (x/(w*128))*(w*128);*/
	}

	int xx = x >> 7;
	int yy = y >> 7;
	int row = (_texelFetch(RAM, 0x8000+8+id*2+yy*4).r<<8)+
				_texelFetch(RAM, 0x8000+8+id*2+yy*4+1).r;
	int bid = _texelFetch(RAM, row+xx).r;
	return getBlock128(bid,x&127,y&127);
}

void main(void)
{
	int cx = (_texelFetch(RAM,0xEE78).r<<8)|_texelFetch(RAM,0xEE79).r;
	int cy = (_texelFetch(RAM,0xEE7C).r<<8)|_texelFetch(RAM,0xEE7D).r;
	int q = int(in_Position.x+cx);
	int w = int(in_Position.y+cy);
	//int qq = q >> 7;
	//int ww = w >> 7;
	
	int ccx = 0;//(_texelFetch(RAM,0xEE8C).r<<8)|_texelFetch(RAM,0xEE8D).r;
	int ccy = (_texelFetch(RAM,0xEE90).r<<8)|_texelFetch(RAM,0xEE91).r;
	int ww = int(in_Position.y+ccy);
	int qq = int(in_Position.x+ccx);

	vec4 A = background(0,q,w); // front plane A
	vec4 B = background(1,qq,ww); // back plane B

	float ad = 0.7-float(int(A.a/4.0))*0.3; // a depth
	float bd = 0.8-float(int(B.a/4.0))*0.3; // b depth
	
	if (A.a == 0 || A.a == 4)
	{
		// if A transparent then move it far away
		ad = 5;
	}
	if (B.a == 0 || B.a == 4)
	{
		// if B transparent then move it far away
		bd = 6;
	}

	if (ad > 1 && bd > 1)
	{
		// backdrop
		out_Color = A;
		gl_FragDepth = 0.9;
		return;
	}

	if (ad <= bd)
	{
		// A in front
		out_Color = A;
		gl_FragDepth = ad;
	}
	else
	{
		// B in front
		out_Color = B;
		gl_FragDepth = bd;
	}
	return;
/*	int q = int(in_Position.x);
	int w = int(in_Position.y);
	int qq = q >> 3;
	int ww = w >> 3;
	
	q = q & (64*8-1);
	w = w & (32*8-1);
	int qq = q >> 3;
	int ww = w >> 3;
	q = q & 7;
	w = w & 7;
	int mi = 0xE000 + qq*2 + ww*128;
	int hw = _texelFetch(VRAM,int(mi)).r;
	int lw = _texelFetch(VRAM,int(mi+1)).r;
	int tid = hw*256+lw;

	out_Color = vec4(getTile(tid, q, w).rgb, 1.0);

	mi = 0xC000 + qq*2 + ww*128;
	hw = _texelFetch(VRAM, mi).r;
	lw = _texelFetch(VRAM, mi+1).r;
	tid = hw*256 + lw;

	vec4 pa = getTile(tid, q, w);
	if (pa.a != 0)
		out_Color = pa;*/
}
