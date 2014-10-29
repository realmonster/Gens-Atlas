// genesis sprite shader
// by r57shell
 
#version 140
 
precision highp float; // needed only for version 1.30

uniform isamplerBuffer PAL;
uniform isamplerBuffer VRAM;
uniform int sprite_entry;

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

void main(void)
{
	int se = sprite_entry;
	int hhvv = se >> 16;
	int hh = (hhvv >> 2)+1;
	int vv = (hhvv & 0x3)+1;
	int x = int(in_Position.x);
	int y = int(in_Position.y);
	int xx = (x >> 3);
	int yy = (y >> 3);
	if ((se & 0x800) != 0)
		xx = hh - 1 - xx;
	if ((se & 0x1000) != 0)
		yy = vv - 1 - yy;
	vec4 c = getTile((se & 0xFFFF) + xx*vv + yy, x&7, y&7);
	if (c.a >= 4)
		gl_FragDepth = 0.3; // sprite low
	else
		gl_FragDepth = 0.6; // sprite high
	if (c.a == 4)
	{
		c.a = 0;
		gl_FragDepth = 0.9; // move far away
	}
	out_Color = c;
}
