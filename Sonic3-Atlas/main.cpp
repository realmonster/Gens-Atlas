
#include <gl/glew.h>
#include <gl/wglew.h>

#include <vector>
#include <algorithm>

#define DLLEXPORT extern "C" __declspec(dllexport) 

HWND ShaderLogHwnd;
HGLRC GLRC = NULL;
GLuint glScreenTexture;

GLuint MainShader;
GLuint SpriteShader;

GLuint myArrayUBO;
GLuint RomTexture;
GLuint RomBuffer;
GLuint PalTexture;
GLuint PalBuffer;
GLuint VRAMTexture;
GLuint VRAMBuffer;
GLuint RAMTexture;
GLuint RAMBuffer; 

GLint CompileShader(GLuint shader, LPCSTR path)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return GL_FALSE;

	fseek(f,0,SEEK_END);
	GLint len = ftell(f);
	fseek(f,0,SEEK_SET);

	GLchar *data = (GLchar*)malloc(len);
	if (!data)
	{
		fclose(f);
		return GL_FALSE;
	}

	int readed = fread(data, 1, len, f);
	fclose(f);
	if (readed != len)
	{
		free(data);
		return GL_FALSE;
	}

	GLchar* strings[1];
	strings[0] = data;
	glShaderSource(shader, 1, (const GLchar**)strings, &len);
	free(data);

	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	return success;
}

std::vector<GLchar> GetShaderLog(GLuint shader)
{
	GLint logSize = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

	std::vector<GLchar> infoLog;
	infoLog.resize(logSize);

	GLsizei len;
	glGetShaderInfoLog(shader, logSize, &len, infoLog.data());

	return infoLog;
}

GLint CompileProgram(GLuint program, GLuint vertex, GLuint fragment)
{
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);

	glLinkProgram(program);

	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	return success;
}

std::vector<GLchar> GetProgramLog(GLuint shader)
{
	GLint logSize = 0;
	glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logSize);

	std::vector<GLchar> infoLog;
	infoLog.resize(logSize);

	GLsizei len = 0;
	glGetProgramInfoLog(shader, logSize, &len, infoLog.data());

	return infoLog;
}

void FilterShaderLog(std::vector<GLchar> *log)
{
	if (!log)
		return;
	for (size_t i=0; i<log->size(); ++i)
	{
		if ((*log)[i] == 0)
		{
			log->erase((*log).begin()+i);
			--i;
		}
		if ((*log)[i] == '\n' && (!i || (*log)[i-1] != '\r'))
		{
			log->insert((*log).begin()+i, '\r');
			++i;
		}
	}
}

GLuint MakeProgram(LPCSTR vert_path, LPCSTR frag_path, std::vector<GLchar> *log)
{
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	GLint res = CompileShader(vertex, vert_path);
	if (log)
	{
		std::vector<GLchar> l = GetShaderLog(vertex);
		log->insert(log->end(),l.begin(),l.end());
	}
	if (res == GL_FALSE)
	{
		glDeleteShader(vertex);
		FilterShaderLog(log);
		return res;
	}

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	res = CompileShader(fragment, frag_path);
	if (log)
	{
		log->push_back('\n');
		std::vector<GLchar> l = GetShaderLog(fragment);
		log->insert(log->end(),l.begin(),l.end());
	}
	if (res == GL_FALSE)
	{
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		FilterShaderLog(log);
		return res;
	}

	GLuint program = glCreateProgram();
	res = CompileProgram(program, vertex, fragment);
	if (log)
	{
		log->push_back('\n');
		std::vector<GLchar> l = GetProgramLog(program);
		log->insert(log->end(),l.begin(),l.end());

		FilterShaderLog(log);
	}

	glDetachShader(program, vertex);
	glDetachShader(program, fragment);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	return program;
}

void InitTextures()
{
	glGenBuffers (1, &RomBuffer);
	glGenTextures(1, &RomTexture);
	glGenBuffers (1, &PalBuffer);
	glGenTextures(1, &PalTexture);
	glGenBuffers (1, &VRAMBuffer);
	glGenTextures(1, &VRAMTexture);
	glGenBuffers (1, &RAMBuffer);
	glGenTextures(1, &RAMTexture);
}

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

static BYTE current_RAM[0x10000];
static BYTE current_VRAM[0x10000];
static WORD current_CRAM[128];

static BYTE* emu_RAM;
static BYTE* emu_VRAM;
static WORD* emu_CRAM;

SafeArray<BYTE> ROM;
SafeArray<BYTE> RAM;
SafeArray<BYTE> VRAM;
SafeArray<WORD> CRAM;
unsigned int *MD_Screen32;
unsigned long *TAB336;

WORD PAL[0x80+0x2000];

#define HSCROLL (PAL+0x80) 
#define HSIZE ((BYTE*)(PAL+0x80+0x1000))

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

DLLEXPORT void Renderer_ROM_Loaded()
{
	glBindBuffer (GL_TEXTURE_BUFFER, RomBuffer);
	glBufferData (GL_TEXTURE_BUFFER, 0x400000, ROM.data, GL_STATIC_DRAW);
}


DLLEXPORT void Renderer_PreRender()
{
	memcpy(current_RAM,emu_RAM,0x10000);
	memcpy(current_VRAM,emu_VRAM,0x10000);
	memcpy(current_CRAM,emu_CRAM,128);
}

void UpdateTextures()
{
	glBindTexture(GL_TEXTURE_BUFFER, RomTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, RomBuffer);

	UpdatePalAndScroll();
	glBindBuffer (GL_TEXTURE_BUFFER, PalBuffer);
	glBufferData (GL_TEXTURE_BUFFER, 0x4100, PAL, GL_STATIC_DRAW);

	glBindTexture(GL_TEXTURE_BUFFER, PalTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, PalBuffer);

	glBindBuffer (GL_TEXTURE_BUFFER, VRAMBuffer);
	glBufferData (GL_TEXTURE_BUFFER, 0x10000, VRAM.data, GL_STATIC_DRAW);

	glBindTexture(GL_TEXTURE_BUFFER, VRAMTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, VRAMBuffer);

	glBindBuffer (GL_TEXTURE_BUFFER, RAMBuffer);
	glBufferData (GL_TEXTURE_BUFFER, 0x10000, RAM.data, GL_STATIC_DRAW);

	glBindTexture(GL_TEXTURE_BUFFER, RAMTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, RAMBuffer);
}

void DelTextures()
{
	glBindTexture(GL_TEXTURE_BUFFER, RomTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, 0);

	glDeleteTextures(1, &RomTexture);
	glDeleteBuffers(1, &RomBuffer);
	
	glBindTexture(GL_TEXTURE_BUFFER, PalTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, 0);

	glDeleteTextures(1, &PalTexture);
	glDeleteBuffers(1, &PalBuffer);
	
	glBindTexture(GL_TEXTURE_BUFFER, VRAMTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, 0);

	glDeleteTextures(1, &VRAMTexture);
	glDeleteBuffers(1, &VRAMBuffer);
	
	glBindTexture(GL_TEXTURE_BUFFER, RAMTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, 0);

	glDeleteTextures(1, &RAMTexture);
	glDeleteBuffers(1, &RAMBuffer);
}

int GetLong(const void *ptr)
{
	unsigned char* d = (unsigned char*)ptr;
	return (d[1]<<24)|(d[0]<<16)|(d[3]<<8)|(d[2]);
}

int GetWord(const void *ptr)
{
	unsigned char* d = (unsigned char*)ptr;
	return (d[1]<<8)|(d[0]);
}

bool CreateGLContext(HDC hDC) 
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int nPixelFormat = ChoosePixelFormat(hDC, &pfd);

	if (nPixelFormat == 0) return false;

	BOOL bResult = SetPixelFormat (hDC, nPixelFormat, &pfd);

	if (!bResult) return false; 

	HGLRC tempContext = wglCreateContext(hDC);
	wglMakeCurrent(hDC, tempContext);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		MessageBox(NULL, "GLEW is not initialized!", "ERROR", MB_OK);
		return false;
	}

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};

	if(wglewIsSupported("WGL_ARB_create_context") == 1)
	{
		GLRC = wglCreateContextAttribsARB(hDC, 0, attribs);
		wglMakeCurrent(NULL,NULL);
		wglDeleteContext(tempContext);
		if (!wglMakeCurrent(hDC, GLRC))
			return false;
	}
	else
	{
		//It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
		GLRC = tempContext;
	}

	//Checking GL version
	const GLubyte *GLVersionString = glGetString(GL_VERSION);

	//Or better yet, use the GL3 way to get the version number
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

	if (!GLRC) return false;

	return true;
}

DLLEXPORT LPCSTR Renderer_Init(HWND hWnd,unsigned int* _MD_Screen32,unsigned long* _TAB336, BYTE* _ROM, BYTE* _RAM, BYTE* _VRAM, WORD* _CRAM)
{
	MD_Screen32 = _MD_Screen32;
	TAB336 = _TAB336;
	emu_RAM = _RAM;
	emu_VRAM = _VRAM;
	emu_CRAM = _CRAM;
	ROM.data = _ROM;          ROM.mask = 0x3fffff;
	RAM.data = current_RAM;   RAM.mask = 0xffff;
	VRAM.data = current_VRAM; VRAM.mask = 0xFFFF;
	CRAM.data = current_CRAM; CRAM.mask = 127;
	HDC dc = GetDC(hWnd);
	if (!CreateGLContext(dc))
	{
		GLRC = NULL;
		return "Error can't create OpenGL context!";
	}
	HWND staticw = CreateWindowEx(NULL, "EDIT", "",
		 WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_VISIBLE,
		0,0,500,500,
		NULL,
		NULL,
		NULL, NULL);
	ShaderLogHwnd = CreateWindowEx(NULL, "EDIT", "",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL | WS_THICKFRAME,
		0,0,500,500,
		staticw,
		NULL,
		NULL, NULL); 

	glGenTextures(1, &glScreenTexture);

	std::vector<GLchar> log;
	std::vector<GLchar> log1;
	MainShader = MakeProgram("vert.glsl", "frag.glsl", &log);
	SpriteShader = MakeProgram("vert.glsl", "sprite.glsl", &log1);
	log.insert(log.end(), log1.begin(), log1.end());
	log.push_back('\0');
	SetWindowText(ShaderLogHwnd, log.data());
	InitTextures();
	ReleaseDC(hWnd, dc);
	return NULL;
}

DLLEXPORT void Renderer_Delete()
{
	if (GLRC)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		DelTextures();
		glDeleteTextures(1, &glScreenTexture);
		glDeleteProgram(MainShader);
		glDeleteProgram(SpriteShader);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(GLRC);
		//MessageBox(NULL, "DELETE", "DELETE", MB_OK);
		HWND staticw = GetParent(ShaderLogHwnd);
		DestroyWindow(ShaderLogHwnd);
		CloseWindow(staticw);
		DestroyWindow(staticw);
	}
}

GLbyte BlitData[1024*1024*3];

void DrawSprite(int id, int x, int y, int size)
{
	GLint res = glGetUniformLocation(SpriteShader, "sprite_entry");
	if (res != -1)
	{
		glUniform1i(res, id|(size<<16)); //Texture unit 0 is for base images.
	}

	res = glGetUniformLocation(SpriteShader, "water_level");
	if (res != -1)
	{
		int water_level = 0x100;
		if (RAM[0xF730^1] != 0)
			water_level = *(WORD*)&RAM[0xF646]-*(WORD*)&RAM[0xEE7C]-y;
		glUniform1i(res, water_level); //Texture unit 0 is for base images.
	}

	float sw = (((size>>2)&3)+1)*8;
	float sh = (((size)&3)+1)*8;

	glBegin(GL_TRIANGLES);

	glColor3f (0.0f, 0.0f, 0.0f);
	glVertex2f(x   , y   );
	glColor3f (0.0f,   sh, 0.0f);
	glVertex2f(x   , y+sh);
	glColor3f (  sw,   sh, 0.0f);
	glVertex2f(x+sw, y+sh);

	glColor3f (0.0f, 0.0f, 0.0f);
	glVertex2f(x   , y   );
	glColor3f (  sw,   sh, 0.0f);
	glVertex2f(x+sw, y+sh);
	glColor3f (  sw, 0.0f, 0.0f);
	glVertex2f(x+sw, y   );

	glEnd();
}

void DrawFrame(int offset, int frame, int flags, int base, int x, int y)
{
	int count = 1;
	if (!(flags&(1<<5)))
	{
		offset += *(short*)&ROM[offset+frame*2];
		count = *(WORD*)&ROM[offset];
		offset += 2;
	}
	if (count > 20)
		count = 20;
	for (int i = count-1; i >= 0; --i)
	{
		int s = ROM[offset+i*6];
		DrawSprite(base+*(WORD*)&ROM[offset+i*6+2]^((flags&3)<<11), // sprite id
					x+((flags&1)?-*(short*)&ROM[offset+i*6+4]-(((s>>2)&3)+1)*8:*(short*)&ROM[offset+i*6+4]), // x
					y+((flags&2)?-(char)ROM[offset+i*6+1]-(((s)&3)+1)*8:(char)ROM[offset+i*6+1]), // y
					s); // size = SS
	}
}

std::vector<int> sprite_priority;
std::vector<int> sprite_indexes;

bool priority_cmp(int a, int b)
{
	if (sprite_priority[a]!=sprite_priority[b])
		return sprite_priority[a]<sprite_priority[b];
	return a<b;
}

extern void DrawObject(int id, int arg, int flags, int x, int y);

DLLEXPORT void Renderer_Render(HWND hWnd, const RECT *RectSrc, const RECT *RectDest)
{
	glDepthRange(-1, 1);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	for (int y=0; y<224; ++y)
	{
		for (int x=0; x<320; ++x)
		{
			unsigned int c = MD_Screen32[TAB336[y] + 8+x];
			GLbyte * d= &BlitData[(y*320+x)*3];
			*(d++) = (GLbyte)((c>>16)&0xFF);
			*(d++) = (GLbyte)((c>>8)&0xFF);
			*(d++) = (GLbyte)((c)&0xFF);
		}
	}

	glBindTexture(GL_TEXTURE_2D, glScreenTexture);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,320,224,0,GL_RGB,GL_UNSIGNED_BYTE,BlitData);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// when texture area is small, bilinear filter the closest MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// when texture area is large, bilinear filter the first MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	// if wrap is true, the texture wraps over at the edges (repeat)
	//       ... false, the texture ends at the edges (clamp)
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	UpdateTextures();

	glViewport(0, 0, RectDest->right-RectDest->left, RectDest->bottom-RectDest->top);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslated(-1.0, 1.0, 0.0);
	glScaled(2.0/(RectDest->right-RectDest->left), -2.0/(RectDest->bottom-RectDest->top), 1.0);

	glTranslated((RectDest->right-RectDest->left-(RectSrc->right-RectSrc->left))/2,
		(RectDest->bottom-RectDest->top-(RectSrc->bottom-RectSrc->top))/2, 0.0);

	glEnable(GL_TEXTURE_2D);
	//glActiveTexture(GL_TEXTURE0);

	glUseProgram(MainShader);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLint res = glGetUniformLocation(MainShader, "ROM");
	if (res != -1)
	{
		glUniform1i(res, 0); //Texture unit 0 is for base images.
	}
	res = glGetUniformLocation(MainShader, "PAL");
	if (res != -1)
	{
		glUniform1i(res, 1); //Texture unit 0 is for base images.
	}
	res = glGetUniformLocation(MainShader, "VRAM");
	if (res != -1)
	{
		glUniform1i(res, 2); //Texture unit 0 is for base images.
	}
	res = glGetUniformLocation(MainShader, "RAM");
	if (res != -1)
	{
		glUniform1i(res, 3); //Texture unit 0 is for base images.
	}

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_BUFFER, RomTexture);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_BUFFER, PalTexture);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_BUFFER, VRAMTexture);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_BUFFER, RAMTexture);

	// Foreground, Background layers
	int w = RectDest->right-RectDest->left;
	int h = RectDest->bottom-RectDest->top;
	glBegin(GL_TRIANGLES);
	glColor3f(-20000.0f, -20000.0f, 0.0f);
	glVertex2i(-20000,  -20000);
	glColor3f(-20000.0f, 20000.0f, 0.0f);
	glVertex2i(-20000, 20000);
	glColor3f(20000, 20000.0f, 0.0f);
	glVertex2i(20000, 20000);

	glColor3f(-20000.0f, -20000.0f, 0.0f);
	glVertex2i(-20000,  -20000);
	glColor3f(20000.0f, 20000.0f, 0.0f);
	glVertex2i(20000, 20000);
	glColor3f(20000.0f, -20000.0f, 0.0f);
	glVertex2i(20000, -20000);
	glEnd();

	glUseProgram(0);

	int ring_current = GetLong(&RAM[0xEE42]);
	int ring_start = 0;
	int ring_end = 0;
	for (int i=ring_current; i>=0 && i > ring_current-4000; i-=4)
		if (GetLong(&ROM[i]) == 0)
		{
			ring_start = i;
			break;
		}

	for (int i=ring_current; i<0x400000 && i < ring_current+4000; i+=4)
		if (ROM[(i)^1] == 0xFF
		 && ROM[(i+1)^1] == 0xFF)
		 {
			ring_end = i;
			break;
		 }

	if (ring_start && ring_end
	&& ring_end - ring_start < 512*4)
	{
		int camerax = GetWord(&RAM[0xEE78]);
		int cameray = GetWord(&RAM[0xEE7C]);

		glUseProgram(SpriteShader);
		GLint res = glGetUniformLocation(SpriteShader, "sprite_entry");
		if (res != -1)
		{
			switch (RAM[0xFEB3^1])
			{
				case 0:
					glUniform1i(res, (0x26BC+0)|(0x50000));
					break;
				case 1:
					glUniform1i(res, (0x26BC+4)|(0x50000));
					break;
				case 2:
					glUniform1i(res, (0x26BC+8)|(0x10000));
					break;
				case 3:
					glUniform1i(res, (0x26BC+4)|(0x50800));
					break;
			}
		}
		res = glGetUniformLocation(SpriteShader, "RAM");
		if (res != -1)
		{
			glUniform1i(res, 3); //Texture unit 0 is for base images.
		}
		res = glGetUniformLocation(SpriteShader, "VRAM");
		if (res != -1)
		{
			glUniform1i(res, 2); //Texture unit 0 is for base images.
		}
		
		res = glGetUniformLocation(SpriteShader, "water_level");
		for (int i=0; ring_start+i<ring_end; i+=4)
		{
			if (GetWord(&RAM[0xE700+(i>>1)]) != 0)
				continue;
			float rx = GetWord(&ROM[ring_start+i]) - camerax;
			float ry = GetWord(&ROM[ring_start+i+2]) - cameray;
			float rw = 8;
			if (RAM[0xFEB3^1] ==  2)
				rw = 4;
			float rh = 8;
			//if (rx < 0 || rx > w)
			//	continue;
			//if (ry < 0 || ry > h)
			//	continue;
			if (res != -1)
			{
				int water_level = 0x100;
				if (RAM[0xF730^1] != 0)
					water_level = *(WORD*)&RAM[0xF646]-*(WORD*)&RAM[0xEE7C]-(ry-rh);
				glUniform1i(res, water_level);
			}
			glBegin(GL_TRIANGLES);

			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex2f(rx-rw, ry-rh);
			glColor3f(0.0f, 16.0f, 0.0f);
			glVertex2f(rx-rw, ry+rh);
			glColor3f(rw*2, 16.0f, 0.0f);
			glVertex2f(rx+rw, ry+rh);

			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex2f(rx-rw, ry-rh);
			glColor3f(rw*2, 16.0f, 0.0f);
			glVertex2f(rx+rw, ry+rh);
			glColor3f(rw*2, 0.0f, 0.0f);
			glVertex2f(rx+rw, ry-rh);

			glEnd();
		}

		glUseProgram(0);
	}

	int object_current = GetLong(&RAM[0xF772]);
	int object_start = 0;
	int object_end = 0;
	for (int i=object_current; i>=0 && i > object_current-800*6; i-=6)
		if (GetLong(&ROM[i+2]) == 0)
		{
			object_start = i;
			break;
		}

	for (int i=object_current; i<0x400000 && i < object_current+800*6; i+=6)
		if (ROM[(i)^1] == 0xFF
		 && ROM[(i+1)^1] == 0xFF)
		 {
			object_end = i;
			break;
		 }
	if (object_start && object_end
	&& object_end - object_start < 800*6)
	{
		int camerax = GetWord(&RAM[0xEE78]);
		int cameray = GetWord(&RAM[0xEE7C]);

		glUseProgram(SpriteShader);
		GLint res = glGetUniformLocation(SpriteShader, "sprite_entry");
		if (res != -1)
		{
			switch (RAM[0xFEB3^1])
			{
				case 0:
					glUniform1i(res, (0x26BC+0)|(0x50000));
					break;
				case 1:
					glUniform1i(res, (0x26BC+4)|(0x50000));
					break;
				case 2:
					glUniform1i(res, (0x26BC+8)|(0x10000));
					break;
				case 3:
					glUniform1i(res, (0x26BC+4)|(0x50800));
					break;
			}
		}
		res = glGetUniformLocation(SpriteShader, "RAM");
		if (res != -1)
		{
			glUniform1i(res, 3); //Texture unit 0 is for base images.
		}
		res = glGetUniformLocation(SpriteShader, "VRAM");
		if (res != -1)
		{
			glUniform1i(res, 2); //Texture unit 0 is for base images.
		}
		
		res = glGetUniformLocation(SpriteShader, "water_level");
		for (int i=0; object_start+i<object_end; i+=6)
		{
			if (GetWord(&RAM[0xEB00+(i/6)])&0x80)
				continue;
			float rx = GetWord(&ROM[object_start+i]) - camerax;
			float ry = (GetWord(&ROM[object_start+i+2])&0xFFF) - cameray;
			DrawObject(ROM[(object_start+i+4)^1],
					ROM[(object_start+i+5)^1],
					(GetWord(&ROM[object_start+i+2])>>13)&3,
					rx,
					ry);
		}

		glUseProgram(0);
	}

	// Sprites in VDP table
	/*int link = 0;
	glUseProgram(SpriteShader);
	std::vector<int> links;
	for (int i=0; i<80; ++i)
	{
		links.push_back(link);
		int so = 0xF800+link*8;
		link = VRAM[(so+3)^1];
		if (link == 0)
			break;
	}

	for (int i = links.size()-1; i>=0; --i)
	{
		link = links[i];
		int so = 0xF800+link*8;
		int size = VRAM[(so+2)^1];
		GLint res = glGetUniformLocation(SpriteShader, "sprite_entry");
		if (res != -1)
		{
			glUniform1i(res, GetWord(&VRAM[so+4])|((size&0xF)<<16)); //Texture unit 0 is for base images.
		}

		int yy = GetWord(&VRAM[so]);
		if (yy == 0)
			continue;
		int xx = GetWord(&VRAM[so+6]);
		if (xx == 0)
			continue;
		float y = yy-0x80;
		float x = xx-0x80;
		float sw = (((size>>2)&3)+1)*8;
		float sh = (((size)&3)+1)*8;

		glBegin(GL_TRIANGLES);

		glColor3f (0.0f, 0.0f, 0.0f);
		glVertex2f(x   , y   );
		glColor3f (0.0f,   sh, 0.0f);
		glVertex2f(x   , y+sh);
		glColor3f (  sw,   sh, 0.0f);
		glVertex2f(x+sw, y+sh);

		glColor3f (0.0f, 0.0f, 0.0f);
		glVertex2f(x   , y   );
		glColor3f (  sw,   sh, 0.0f);
		glVertex2f(x+sw, y+sh);
		glColor3f (  sw, 0.0f, 0.0f);
		glVertex2f(x+sw, y   );

		glEnd();
	}
	glUseProgram(0);
	*/

	// Objects in Status Table
	{
		glUseProgram(SpriteShader);

		res = glGetUniformLocation(SpriteShader, "RAM");
		if (res != -1)
		{
			glUniform1i(res, 3); //Texture unit 0 is for base images.
		}
		res = glGetUniformLocation(SpriteShader, "VRAM");
		if (res != -1)
		{
			glUniform1i(res, 2); //Texture unit 0 is for base images.
		}
		sprite_priority.clear();
		sprite_indexes.clear();
			
		for (int i = 0xB000; i<0xCFCB; i+=0x4A)
		{
			sprite_indexes.push_back(sprite_priority.size());
			sprite_priority.push_back(*(WORD*)&RAM[i+8]);
		}
		std::sort(sprite_indexes.begin(),sprite_indexes.end(),priority_cmp);
		int camerax = GetWord(&RAM[0xEE78]);
		int cameray = GetWord(&RAM[0xEE7C]);
		for (int z = sprite_indexes.size(); z>=0; --z)
		{
			int i = 0xB000 + sprite_indexes[z]*0x4A;
			if (RAM[i] == 0
			&& RAM[i+1] == 0
			&& RAM[i+2] == 0
			&& RAM[i+3] == 0)
				continue;
			int flags = RAM[(i+4)^1];
			if (flags & (1<<6)) // compound
			{
				int count = *(WORD*)&RAM[i+0x16];
				if (count > 20)
					count = 20;
				for (int j=count-1; j>=0; --j)
					DrawFrame(GetLong(&RAM[i+0xC]), // offset
						RAM[i+0x18+j*6+4], // frame
						flags, // flags
						*(WORD*)&RAM[i+0xA], // base
						(*(short*)&RAM[i+0x10])*0-camerax+*(short*)&RAM[i+0x18+j*6],
						(*(short*)&RAM[i+0x14])*0-cameray+*(short*)&RAM[i+0x18+j*6+2]);
			}
			DrawFrame(GetLong(&RAM[i+0xC]), // offset
				RAM[(i+0x22)^1], // frame
				flags, // flags
				*(WORD*)&RAM[i+0xA], // base
				*(short*)&RAM[i+0x10]-camerax,
				*(short*)&RAM[i+0x14]-cameray);
		}
		glUseProgram(0);
	}

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	// Genesis Screen
	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0, 1.0, 1.0f);
	glBindTexture(GL_TEXTURE_2D, glScreenTexture);

	glBegin(GL_TRIANGLES);
	glTexCoord3f(0.0f, 1.0f, 0.0f);
	glVertex2i(0, 224);
	glTexCoord3f(0.0f, 0.0f, 0.0f);
	glVertex2i(0, 0);
	glTexCoord3f(1.0f, 0.0f, 0.0f);
	glVertex2i(320, 0);

	glTexCoord3f(0.0f, 1.0f, 0.0f);
	glVertex2i(0,  224);
	glTexCoord3f(1.0f, 0.0f, 0.0f);
	glVertex2i(320, 0);
	glTexCoord3f(1.0f, 1.0f, 0.0f);
	glVertex2i(320, 224);
	glEnd();
	
	HDC dc = GetDC(hWnd);
	SwapBuffers(dc);
	ReleaseDC(hWnd, dc);
}
