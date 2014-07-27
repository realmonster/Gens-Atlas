#include <stdio.h>
#include <math.h>
#include "G_ddraw.h"
#include "G_dsound.h"
#include "G_Input.h"
#include "G_main.h"
#include "resource.h"
#include "gens.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "misc.h"
#include "blit.h"
#include "scrshot.h"
#include "net.h"
#include "save.h"

#include "cdda_mp3.h"

#include "movie.h"
#include "moviegfx.h"
#include "io.h"
#include "hackscommon.h"
#include "drawutil.h"
#include "luascript.h"

#include <gl/glew.h>
#include <gl/wglew.h>

#include <vector>

HWND ShaderLogHwnd;
HGLRC GLRC;
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

int Update_Rom_Buffer()
{
	glBindBuffer (GL_TEXTURE_BUFFER, RomBuffer);
	glBufferData (GL_TEXTURE_BUFFER, 0x400000, Rom_Data, GL_STATIC_DRAW);
	return 0;
}

void UpdateTextures()
{
	glBindTexture(GL_TEXTURE_BUFFER, RomTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, RomBuffer);

	glBindBuffer (GL_TEXTURE_BUFFER, PalBuffer);
	glBufferData (GL_TEXTURE_BUFFER, 0x80, CRam, GL_STATIC_DRAW);

	glBindTexture(GL_TEXTURE_BUFFER, PalTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, PalBuffer);

	glBindBuffer (GL_TEXTURE_BUFFER, VRAMBuffer);
	glBufferData (GL_TEXTURE_BUFFER, 0x10000, VRam, GL_STATIC_DRAW);

	glBindTexture(GL_TEXTURE_BUFFER, VRAMTexture);
	glTexBuffer (GL_TEXTURE_BUFFER, GL_R8UI, VRAMBuffer);

	glBindBuffer (GL_TEXTURE_BUFFER, RAMBuffer);
	glBufferData (GL_TEXTURE_BUFFER, 0x10000, Ram_68k, GL_STATIC_DRAW);

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

int GetLong(void *ptr)
{
	unsigned char* d = (unsigned char*)ptr;
	return (d[1]<<24)|(d[0]<<16)|(d[3]<<8)|(d[2]);
}

int GetWord(void *ptr)
{
	unsigned char* d = (unsigned char*)ptr;
	return (d[1]<<8)|(d[0]);
} 

clock_t Last_Time = 0, New_Time = 0;
clock_t Used_Time = 0;

int Flag_Clr_Scr = 0;
int Sleep_Time = 1;
int FS_VSync;
int W_VSync;
int Res_X = 320 << (int) (Render_FS > 0); //Upth-Add - For the new configurable
int Res_Y = 240 << (int) (Render_FS > 0); //Upth-Add - fullscreen resolution (defaults 320 x 240)
bool FS_No_Res_Change = false; //Upth-Add - For the new fullscreen at same resolution
int Stretch; 
int Blit_Soft;
int Effect_Color = 7;
int FPS_Style = EMU_MODE | WHITE;
int Message_Style = EMU_MODE | WHITE | SIZE_X2;
int Kaillera_Error = 0;
unsigned char CleanAvi = 1;
extern "C" int disableSound, disableSound2, disableRamSearchUpdate;

long int MovieSize;//Modif
int SlowFrame=0; //Modif

static char Info_String[1024] = "";
static int Message_Showed = 0;
static unsigned int Info_Time = 0;

void (*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
void (*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);
int (*Update_Frame)();
int (*Update_Frame_Fast)();

int Correct_256_Aspect_Ratio = 1;

// if you have to debug something in fullscreen mode
// but the fullscreen lock prevents you from seeing the debugger
// when a breakpoint/assertion/crash/whatever happens,
// then it might help to define this.
// absolutely don't leave it enabled by default though, not even in _DEBUG
//#define DISABLE_EXCLUSIVE_FULLSCREEN_LOCK


//#define MAPHACK
#if ((!defined SONICMAPHACK) && (!defined SONICROUTEHACK)) 
#define Update_RAM_Cheats(); 
#else
#include "SonicHackSuite.h"
#endif

#ifdef SONICCAMHACK
#include "SonicHackSuite.h"
#elif defined RKABOXHACK
#include "RKABoxHack.h"
#elif (defined ECCOBOXHACK) || (defined ECCO1BOXHACK)
#include "EccoBoxHack.h"
#endif

int Update_Frame_Adjusted()
{
	if(disableVideoLatencyCompensationCount)
		disableVideoLatencyCompensationCount--;

	if(!IsVideoLatencyCompensationOn())
	{
		// normal update
		return Update_Frame();
	}
	else
	{
		// update, and render the result that's some number of frames in the (emulated) future
		// typically the video takes 2 frames to catch up with where the game really is,
		// so setting VideoLatencyCompensation to 2 can make the input more responsive
		//
		// in a way this should actually make the emulation more accurate, because
		// the delay from your computer hardware stacks with the delay from the emulated hardware,
		// so eliminating some of that delay should make it feel closer to the real system

		disableSound2 = true;
		int retval = Update_Frame_Fast();
		Update_RAM_Search();
		disableRamSearchUpdate = true;
		Save_State_To_Buffer(State_Buffer);
		for(int i = 0; i < VideoLatencyCompensation-1; i++)
			Update_Frame_Fast();
		disableSound2 = false;
		Update_Frame();
		disableRamSearchUpdate = false;
		Load_State_From_Buffer(State_Buffer);
		return retval;
	}
}

int Update_Frame_Hook()
{
	// note: we don't handle LUACALL_BEFOREEMULATION here
	// because it needs to run immediately before SetCurrentInputCondensed() might get called
	// in order for joypad.get() and joypad.set() to work as expected

	#ifdef SONICCAMHACK
	int retval = SonicCamHack();
	#else
		int retval = Update_Frame_Adjusted();

		#ifdef RKABOXHACK
			RKADrawBoxes();
			CamX = CheatRead<short>(0xFFB158);
			CamY = CheatRead<short>(0xFFB1D6);
		#elif (defined ECCOBOXHACK) || (defined ECCO1BOXHACK) 
			unsigned char curlev = CheatRead<unsigned char>(0xFFA7E0);
			static int PrevX = 0,PrevY = 0;
			int xpos,ypos;
//			switch (CheatRead<unsigned char>(0xFFA555))
//			{
//				case 0x20:
//				case 0x28:
//				case 0xAC:
					EccoDrawBoxes();
					xpos = CheatRead<int>(0xFFA8F0);
					ypos = CheatRead<int>(0xFFA8F4);
//					break;
//				case 0xF6:
//					EccoDraw3D();
//					xpos = ypos = CheatRead<int>(0xFFB13E);
//					break;
//				default:
//					xpos = PrevX + 1;
//					ypos = PrevY + 1;
//			}
//			if ((xpos == PrevX) && (ypos == PrevY) && CheatRead<int>(0xFFAA32)) Lag_Frame = 1;
//			else	PrevX = xpos,PrevY = ypos;
			CamX = CheatRead<short>(CAMXPOS);
			CamY = CheatRead<short>(CAMYPOS);
		#endif
	#endif

	CallRegisteredLuaFunctions(LUACALL_AFTEREMULATION);

	CallRegisteredLuaFunctions(LUACALL_AFTEREMULATIONGUI);

	return retval;
}
int Update_Frame_Fast_Hook()
{
	// note: we don't handle LUACALL_BEFOREEMULATION here
	// because it needs to run immediately before SetCurrentInputCondensed() might get called
	// in order for joypad.get() and joypad.set() to work as expected

	int retval = Update_Frame_Fast();

	CallRegisteredLuaFunctions(LUACALL_AFTEREMULATION);
	Update_RAM_Search();
	return retval;
}

void Put_Info_NonImmediate(char *Message, int Duration)
{
	if (Show_Message)
	{
		strcpy(Info_String, Message);
		Info_Time = GetTickCount() + Duration;
		Message_Showed = 1;
	}
}

void Put_Info(char *Message, int Duration)
{
	if (Show_Message)
	{
		Do_VDP_Refresh();
		Put_Info_NonImmediate(Message);

		// Modif N. - in case game is paused or at extremely low speed, update screen on new message
		extern bool g_anyScriptsHighSpeed;
		if(!Setting_Render && !g_anyScriptsHighSpeed)
		{
			extern HWND HWnd;

			if(FrameCount == 0) // if frame count is 0 it's OK to clear the screen first so we can see the message better
			{
				memset(MD_Screen, 0, sizeof(MD_Screen));
				memset(MD_Screen32, 0, sizeof(MD_Screen32));
			}

			Flip(HWnd);
		}
	}
}


int Init_Fail(HWND hwnd, char *err)
{
	End_DDraw();
	MessageBox(hwnd, err, "Oups ...", MB_OK);

	return 0;
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


int Init_DDraw(HWND hWnd)
{
	int Rend;

	int oldBits32 = Bits32;

	End_DDraw();

	if (Full_Screen) Rend = Render_FS;
	else Rend = Render_W;

	HDC dc = GetDC(hWnd);
	if (!CreateGLContext(dc))
	{
		return Init_Fail(hWnd, "Error can't create OpenGL context!");
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
	SetWindowText(ShaderLogHwnd, log.data());
	InitTextures();
	ReleaseDC(hWnd, dc);

	if (Res_X < (320 << (int) (Render_FS > 0))) Res_X = 320 << (int) (Render_FS > 0); //Upth-Add - Set a floor for the resolution
    if (Res_Y < (240 << (int) (Render_FS > 0))) Res_Y = 240 << (int) (Render_FS > 0); //Upth-Add - 320x240 for single, 640x480 for double and other

	//if (Full_Screen && Render_FS >= 2) //Upth-Modif - Since software blits don't stretch right, we'll use 640x480 for those render modes
	// Modif N. removed the forced 640x480 case because it caused "windowed fullscreen mode" to be ignored and because I fixed the fullscreen software blit stretching

	if (Full_Screen && !(FS_No_Res_Change))
	{
		//if (FAILED(lpDD->SetDisplayMode(Res_X, Res_Y, 16, 0, 0)))
		//	return Init_Fail(hWnd, "Error with lpDD->SetDisplayMode !");
	}

	// make sure the render mode is still valid (changing options in a certain order can make it invalid at this point)
	Set_Render(hWnd, Full_Screen, -1, false);

	// make sure the menu reflects the current mode (which it generally won't yet if we changed to/from 32-bit mode in this function)
	Build_Main_Menu();

	return 1;
}

void End_DDraw()
{
	if (GLRC)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &glScreenTexture);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(GLRC);
	}
}


HRESULT RestoreGraphics(HWND hWnd)
{
	/*HRESULT rval1 = lpDDS_Primary->Restore();
	HRESULT rval2 = lpDDS_Back->Restore();

	// Modif N. -- fixes lost surface handling when the color depth has changed
	if (rval1 == DDERR_WRONGMODE || rval2 == DDERR_WRONGMODE)
		return Init_DDraw(hWnd) ? DD_OK : DDERR_GENERIC;

	return SUCCEEDED(rval2) ? rval1 : rval2;*/
	return 0;
}


int Clear_Primary_Screen(HWND hWnd)
{
	if(!GLRC)
		return 0; // bail if directdraw hasn't been initialized yet or if we're still in the middle of initializing it

	glClear(GL_COLOR_BUFFER_BIT);
	HDC dc = GetDC(hWnd);
	SwapBuffers(dc);
	ReleaseDC(hWnd,dc);
	return 1;
}


int Clear_Back_Screen(HWND hWnd)
{
	if(!GLRC)
		return 0; // bail if directdraw hasn't been initialized yet or if we're still in the middle of initializing it

	//glClear(GL_COLOR_BUFFER_BIT);

	return 1;
}


void Restore_Primary(void)
{
	/*if (lpDD && Full_Screen && FS_VSync)
	{
		while (lpDDS_Primary->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_SURFACEBUSY);
		lpDD->FlipToGDISurface();
	}*/
}

// Calculate Draw Area
//  inputs: hWnd
// outputs:
//    RectDest: destination rect on screen
//    RectSrc: source rect in back buffer used for software blit
//             and for hardware blit from Back surface into destination (Primary or its Flip surface)
void CalculateDrawArea(HWND hWnd, RECT& RectDest, RECT& RectSrc)
{
	POINT p;
	float Ratio_X, Ratio_Y;
	
	int FS_X,FS_Y; //Upth-Add - So we can set the fullscreen resolution to the current res without changing the value that gets saved to the config
	int Render_Mode;
	if (Full_Screen)
	{
		Render_Mode = Render_FS;

		if (FS_No_Res_Change) { //Upth-Add - If we didn't change resolution when we went Full Screen
			DEVMODE temp;
			EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&temp); //Upth-Add - Gets the current screen resolution
			FS_X = temp.dmPelsWidth;
			FS_Y = temp.dmPelsHeight;
		}
		else { //Upth-Add - Otherwise use the configured resolution values
			FS_X = Res_X; 
			FS_Y = Res_Y;
		}
	}
	else
	{
		Render_Mode = Render_W;

		GetClientRect(hWnd, &RectDest);
		FS_X = RectDest.right;
		FS_Y = RectDest.bottom;
	}

	Ratio_X = (float) FS_X / 320.0f; //Upth-Add - Find the current size-ratio on the x-axis
	Ratio_Y = (float) FS_Y / 240.0f; //Upth-Add - Find the current size-ratio on the y-axis
	Ratio_X = Ratio_Y = (Ratio_X < Ratio_Y) ? Ratio_X : Ratio_Y; //Upth-Add - Floor them to the smaller value for correct ratio display

	int Screen_X = FULL_X_RESOLUTION; // Genesis screen width
	int Screen_Y = VDP_Num_Vis_Lines; // Genesis screen height

	RectSrc.left = 0 + 8;
	RectSrc.right = Screen_X + 8;
	RectSrc.top = 0;
	RectSrc.bottom = Screen_Y;

	if (Correct_256_Aspect_Ratio)
	{
		RectDest.left = (int) ((FS_X - (320 * Ratio_X))/2); //Upth-Modif - Centering the screen left-right
		RectDest.right = (int) (320 * Ratio_X + RectDest.left); //Upth-modif - again
	}
	else
	{
		RectDest.left = (int) ((FS_X - (Screen_X * Ratio_X))/2); //Upth-Add - Centering left-right
		RectDest.right = (int) (Screen_X * Ratio_X + RectDest.left); //Upth-modif - again
	}

	RectDest.top = (int) ((FS_Y - (Screen_Y * Ratio_Y))/2); //Upth-Modif - centering top-bottom under other circumstances
	RectDest.bottom = (int) (Screen_Y * Ratio_Y) + RectDest.top; //Upth-Modif - using the same method

	if (Stretch)
	{
		RectDest.left = 0;
		RectDest.right = FS_X; //Upth-Modif - use the user configured value
		RectDest.top = 0;      //Upth-Add - also, if we have stretch enabled
		RectDest.bottom = FS_Y;//Upth-Add - we don't correct the screen ratio
	}

	if (Render_Mode >1)
	{
		RectSrc.left *= 2;
		RectSrc.top *= 2;
		RectSrc.right *= 2;
		RectSrc.bottom *= 2;
	}

	if (!Full_Screen)
	{
		p.x = p.y = 0;
		ClientToScreen(hWnd, &p);

		RectDest.top += p.y; //Upth-Modif - this part moves the picture into the window
		RectDest.bottom += p.y; //Upth-Modif - I had to move it after all of the centering
		RectDest.left += p.x;   //Upth-Modif - because it modifies the values
		RectDest.right += p.x;  //Upth-Modif - that I use to find the center
	}
}

extern "C" { extern unsigned int LED_Status;}
void Draw_SegaCD_LED()
{
	if (LED_Status & 2)
	{
		MD_Screen[336 * 220 + 12] = 0x03E0;
		MD_Screen[336 * 220 + 13] = 0x03E0;
		MD_Screen[336 * 220 + 14] = 0x03E0;
		MD_Screen[336 * 220 + 15] = 0x03E0;
		MD_Screen[336 * 222 + 12] = 0x03E0;
		MD_Screen[336 * 222 + 13] = 0x03E0;
		MD_Screen[336 * 222 + 14] = 0x03E0;
		MD_Screen[336 * 222 + 15] = 0x03E0;
		MD_Screen32[336 * 220 + 12] = 0x00FF00;
		MD_Screen32[336 * 220 + 13] = 0x00FF00;
		MD_Screen32[336 * 220 + 14] = 0x00FF00;
		MD_Screen32[336 * 220 + 15] = 0x00FF00;
		MD_Screen32[336 * 222 + 12] = 0x00FF00;
		MD_Screen32[336 * 222 + 13] = 0x00FF00;
		MD_Screen32[336 * 222 + 14] = 0x00FF00;
		MD_Screen32[336 * 222 + 15] = 0x00FF00;
	}

	if (LED_Status & 1)
	{
		MD_Screen[336 * 220 + 12 + 8] = 0xF800;
		MD_Screen[336 * 220 + 13 + 8] = 0xF800;
		MD_Screen[336 * 220 + 14 + 8] = 0xF800;
		MD_Screen[336 * 220 + 15 + 8] = 0xF800;
		MD_Screen[336 * 222 + 12 + 8] = 0xF800;
		MD_Screen[336 * 222 + 13 + 8] = 0xF800;
		MD_Screen[336 * 222 + 14 + 8] = 0xF800;
		MD_Screen[336 * 222 + 15 + 8] = 0xF800;
		MD_Screen32[336 * 220 + 12 + 8] = 0xFF0000;
		MD_Screen32[336 * 220 + 13 + 8] = 0xFF0000;
		MD_Screen32[336 * 220 + 14 + 8] = 0xFF0000;
		MD_Screen32[336 * 220 + 15 + 8] = 0xFF0000;
		MD_Screen32[336 * 222 + 12 + 8] = 0xFF0000;
		MD_Screen32[336 * 222 + 13 + 8] = 0xFF0000;
		MD_Screen32[336 * 222 + 14 + 8] = 0xFF0000;
		MD_Screen32[336 * 222 + 15 + 8] = 0xFF0000;
	}
}

// text, recording status, etc.
void DrawInformationOnTheScreen()
{
	int i;
	int n,j,m,l[3]; //UpthModif - Added l[3] for use when displaying input
	char FCTemp[256],pos;
	static float FPS = 0.0f, frames[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	static unsigned int old_time = 0, view_fps = 0, index_fps = 0, freq_cpu[2] = {0, 0};
	unsigned int new_time[2];

	if (Show_LED && SegaCD_Started)
		Draw_SegaCD_LED();

	if (MainMovie.Status == MOVIE_RECORDING)
	{
		static unsigned int CircleFrameCount =0;
		if((CircleFrameCount++)%64 < 32)
		{
			m=0;
			n=70560+309;
			for(j=0;j<9;j++,n+=327)
			{
				for(i=0;i<9;i++,m++,n++)
				{
					if (RCircle[m]>0) MD_Screen[n]=RCircle[m];
					if (RCircle32[m]>0) MD_Screen32[n]=RCircle32[m];
				}
			}
		}
	}
#ifdef SONICSPEEDHACK 
#include "sonichacksuite.h"
	// SONIC THE HEDGEHOG SPEEDOMETER HACK
	SonicSpeedTicker();
#endif
	//Framecounter
	if(FrameCounterEnabled)
	{
		if(FrameCounterFrames) //Nitsuja added this option, raw integer frame number display
		{
			if(!MainMovie.File || MainMovie.Status == MOVIE_RECORDING)
				sprintf(FCTemp, "%d", FrameCount);
			else
				sprintf(FCTemp, "%d/%d", FrameCount, MainMovie.LastFrame);
		}
		else
		{
			const int fps = CPU_Mode ? 50 : 60;
			if(!MainMovie.File || MainMovie.Status == MOVIE_RECORDING)
				sprintf(FCTemp,"%d:%02d:%02d",(FrameCount/(fps*60)),(FrameCount/fps)%60,FrameCount%fps); //Nitsuja modified this to use the FPS value instead of 60 | Upth-Modif - Nitsuja forgot that even in PAL mode, it's 60 seconds per minute
			else
				sprintf(FCTemp,"%d:%02d:%02d/%d:%02d:%02d",(FrameCount/(fps*60)),(FrameCount/fps)%60,FrameCount%fps, (MainMovie.LastFrame/(fps*60)),(MainMovie.LastFrame/fps)%60,MainMovie.LastFrame%fps);
		}

		unsigned int slashPos = ~0;
		const char* slash = strchr(FCTemp, '/');
		if(slash)
			slashPos = slash - FCTemp;

		if(strlen(FCTemp)>0)
		{
			n=FrameCounterPosition;
			if (FrameCounterPosition==FRAME_COUNTER_TOP_RIGHT || FrameCounterPosition==FRAME_COUNTER_BOTTOM_RIGHT)
			{
				if (!IS_FULL_X_RESOLUTION)
					n-=64;
				int off = strlen(FCTemp) - 8;
				if (off > 0) n-=off*6;
			}
			BOOL drawRed = Lag_Frame;
			for(pos=0;(unsigned int)pos<strlen(FCTemp);pos++)
			{
				if((unsigned int)pos == slashPos)
					drawRed = (MainMovie.Status != MOVIE_PLAYING);

				m=(FCTemp[pos]-'-')*30;

				for(j=0;j<7;j++,n+=330)
				{
					for(i=0;i<6;i++,n++)
					{
						if(j>0 && j<6)
						{
							if (Bits32) MD_Screen32[n]=FCDigit32[m++] & (drawRed?0xFF0000:0xFFFFFF);
							else MD_Screen[n]=FCDigit[m++] & (drawRed?0xF800:0xFFFF);
						}
						else
						{
							if (Bits32) MD_Screen32[n]=0x0000;
							else MD_Screen[n]=0x0000;
						}
						MD_Screen32[n] |= 0xFF000000; // alpha added (opaque)
					}
				}
				n-=336*7-6;
			}
		}
	}
	//Lagcounter
	if(LagCounterEnabled)
	{
		if(LagCounterFrames) //Nitsuja added this option, raw integer Lag number display
		{
			sprintf(FCTemp,"%d",LagCount);
		}
		else
		{
			const int fps = CPU_Mode ? 50 : 60;
			sprintf(FCTemp,"%d:%02d:%02d",(LagCount/(fps*60)),(LagCount/fps)%60,LagCount%fps); //Nitsuja modified this to use the FPS value instead of 60 | Upth-Modif - Nitsuja forgot that even in PAL mode, it's 60 seconds per minute
		}

		if(strlen(FCTemp)>0)
		{
			n=FrameCounterPosition+2352;
			if (!IS_FULL_X_RESOLUTION && (FrameCounterPosition==FRAME_COUNTER_TOP_RIGHT || FrameCounterPosition==FRAME_COUNTER_BOTTOM_RIGHT))
				n-=64;
			for(pos=0;(unsigned int)pos<strlen(FCTemp);pos++)
			{
				m=(FCTemp[pos]-'-')*30;

				for(j=0;j<7;j++,n+=330)
				{
					for(i=0;i<6;i++,n++)
					{
						if(j>0 && j<6)
						{
							if (Bits32) MD_Screen32[n]=FCDigit32[m++] & 0xFF0000;
							else MD_Screen[n]=FCDigit[m++] & 0xF800;
						}
						else
						{
							if (Bits32) MD_Screen32[n]=0x0000;
							else MD_Screen[n]=0x0000;
						}
						MD_Screen32[n] |= 0xFF000000; // alpha added (opaque)
					}
				}
				n-=336*7-6;
			}
		}
	}
	//Show Input
	if(ShowInputEnabled)
	{

		strcpy(FCTemp,"[\\]^ABCSXYZM[\\]^ABCSXYZM[\\]^ABCSXYZM"); //upthmodif

		if(Controller_1_Up) FCTemp[0]='@';
		if(Controller_1_Down) FCTemp[1]='@';
		if(Controller_1_Left) FCTemp[2]='@';
		if(Controller_1_Right) FCTemp[3]='@';
		if(Controller_1_A) FCTemp[4]='@';
		if(Controller_1_B) FCTemp[5]='@';
		if(Controller_1_C) FCTemp[6]='@';
		if(Controller_1_Start) FCTemp[7]='@';
		if(Controller_1_X) FCTemp[8]='@';
		if(Controller_1_Y) FCTemp[9]='@';
		if(Controller_1_Z) FCTemp[10]='@';
		if(Controller_1_Mode) FCTemp[11]='@';
		//Upth - added 3 controller display support
		if (Controller_1_Type & 0x10) {
			if(Controller_1B_Up) FCTemp[12+0]='@';
			if(Controller_1B_Down) FCTemp[12+1]='@';
			if(Controller_1B_Left) FCTemp[12+2]='@';
			if(Controller_1B_Right) FCTemp[12+3]='@';
			if(Controller_1B_A) FCTemp[12+4]='@';
			if(Controller_1B_B) FCTemp[12+5]='@';
			if(Controller_1B_C) FCTemp[12+6]='@';
			if(Controller_1B_Start) FCTemp[12+7]='@';
			if(Controller_1B_X) FCTemp[12+8]='@';
			if(Controller_1B_Y) FCTemp[12+9]='@';
			if(Controller_1B_Z) FCTemp[12+10]='@';
			if(Controller_1B_Mode) FCTemp[12+11]='@';
		}
		else {
			if(Controller_2_Up) FCTemp[12+0]='@';
			if(Controller_2_Down) FCTemp[12+1]='@';
			if(Controller_2_Left) FCTemp[12+2]='@';
			if(Controller_2_Right) FCTemp[12+3]='@';
			if(Controller_2_A) FCTemp[12+4]='@';
			if(Controller_2_B) FCTemp[12+5]='@';
			if(Controller_2_C) FCTemp[12+6]='@';
			if(Controller_2_Start) FCTemp[12+7]='@';
			if(Controller_2_X) FCTemp[12+8]='@';
			if(Controller_2_Y) FCTemp[12+9]='@';
			if(Controller_2_Z) FCTemp[12+10]='@';
			if(Controller_2_Mode) FCTemp[12+11]='@';
		}
		if(Controller_1C_Up) FCTemp[12+12+0]='@';
		if(Controller_1C_Down) FCTemp[12+12+1]='@';
		if(Controller_1C_Left) FCTemp[12+12+2]='@';
		if(Controller_1C_Right) FCTemp[12+12+3]='@';
		if(Controller_1C_A) FCTemp[12+12+4]='@';
		if(Controller_1C_B) FCTemp[12+12+5]='@';
		if(Controller_1C_C) FCTemp[12+12+6]='@';
		if(Controller_1C_Start) FCTemp[12+12+7]='@';
		if(Controller_1C_X) FCTemp[12+12+8]='@';
		if(Controller_1C_Y) FCTemp[12+12+9]='@';
		if(Controller_1C_Z) FCTemp[12+12+10]='@';
		if(Controller_1C_Mode) FCTemp[12+12+11]='@';
		n=FrameCounterPosition-2352;
		if (!IS_FULL_X_RESOLUTION && (FrameCounterPosition==FRAME_COUNTER_TOP_RIGHT || FrameCounterPosition==FRAME_COUNTER_BOTTOM_RIGHT))
			n-=64;
		for(pos=0;pos<12;pos++) //upthmodif
		{
			l[0]=(FCTemp[pos]-'@')*20+420;
			l[1]=(FCTemp[pos+12]-'@')*20+420;
			l[2]=(FCTemp[pos+12+12]-'@')*20+420;

			for(j=0;j<7;j++,n+=332)
			{
				for(i=0;i<4;i++,n++)
				{
					if(j>0 && j<6) {
						if (Controller_1_Type & 0x10)
						{
							if (Bits32) MD_Screen32[n]=(FCDigit32[l[0]++] & FCColor32[0]) | (FCDigit32[l[1]++] & FCColor32[1]) | (FCDigit32[l[2]++] & FCColor32[2]); //Upthmodif - three player display support - three player mode
							else MD_Screen[n]=(FCDigit[l[0]++] & FCColor[Mode_555 & 1][0]) | (FCDigit[l[1]++] & FCColor[Mode_555 & 1][1]) | (FCDigit[l[2]++] & FCColor[Mode_555 & 1][2]); //Upthmodif - three player display support - three player mode
						}
						else 
						{
							if (Bits32) MD_Screen32[n]=(FCDigit32[l[0]++] & FCColor32[3]) | (FCDigit32[l[1]++] & FCColor32[4]); //Upthmodif - three player display support - three player mode
							else MD_Screen[n]=(FCDigit[l[0]++] & FCColor[Mode_555 & 1][3]) | (FCDigit[l[1]++] & FCColor[Mode_555 & 1][4]); //Upthmodif - three player display support - two player mode
						}
					}
					else
					{
						if (Bits32) MD_Screen32[n]=0x0000;
						else MD_Screen[n]=0x0000;
					}
					MD_Screen32[n] |= 0xFF000000; // alpha added (opaque)
				}
			}
			n-=336*7-4;
		}
	
	}

	//// original SONIC THE HEDGEHOG 3 SPEEDOMETER HACK
	//{
	//	char temp [128];
	//	extern unsigned char Ram_68k[64 * 1024];
	//	// sonic 1: D010 ... sonic 2: B010 ... sonic 3: B018
	//	short xvel = (short)(Ram_68k[0xB018] | (Ram_68k[0xB018 + 1] << 8));
	//	short yvel = (short)(Ram_68k[0xB01A] | (Ram_68k[0xB01A + 1] << 8));
	//	short xvel2 = (short)(Ram_68k[0xB062] | (Ram_68k[0xB062 + 1] << 8)); // tails
	//	short yvel2 = (short)(Ram_68k[0xB064] | (Ram_68k[0xB064 + 1] << 8));
	//	sprintf(temp, "(%d, %d) [%d, %d]", xvel,yvel, xvel2,yvel2);
	//	Message_Showed = 1;
	//	if(GetTickCount() > Info_Time)
	//	{
	//		Info_Time = 0x7fffffff;
	//		strcpy(Info_String, "");
	//	}
	//	char temp2 [1024];
	//	char* paren = strchr(Info_String, ']');
	//	strcpy(temp2, paren ? (paren+2) : Info_String);
	//	sprintf(Info_String, "%s %s", temp, temp2);
	//}

	if (Message_Showed)
	{
		if (GetTickCount() > Info_Time)
		{
			Message_Showed = 0;
			strcpy(Info_String, "");
		}
		else //Modif N - outlined message text, so it isn't unreadable on any background color:
		{
			if(!(Message_Style & TRANS))
			{
				int backColor = (((Message_Style & (BLUE|GREEN|RED)) == BLUE) ? RED : BLUE) | (Message_Style & SIZE_X2) | TRANS;
				const static int xOffset [] = {-1,-1,-1,0,1,1,1,0};
				const static int yOffset [] = {-1,0,1,1,1,0,-1,-1};
				for(int i = 0 ; i < 8 ; i++)
					Print_Text(Info_String, strlen(Info_String), 10+xOffset[i], 210+yOffset[i], backColor);
			}
			Print_Text(Info_String, strlen(Info_String), 10, 210, Message_Style);
		}

	}
	else if (Show_FPS)
	{	
		if (freq_cpu[0] > 1)				// accurate timer ok
		{
			if (++view_fps >= 16)
			{
				QueryPerformanceCounter((union _LARGE_INTEGER *) new_time);

				if (new_time[0] != old_time)
				{					
					FPS = (float) (freq_cpu[0]) * 16.0f / (float) (new_time[0] - old_time);
					sprintf(Info_String, "%.1f", FPS);
				}
				else
				{
					sprintf(Info_String, "too much...");
				}

				old_time = new_time[0];
				view_fps = 0;
			}
		}
		else if (freq_cpu[0] == 1)			// accurate timer not supported
		{
			if (++view_fps >= 10)
			{
				new_time[0] = GetTickCount();
		
				if (new_time[0] != old_time) frames[index_fps] = 10000 / (float)(new_time[0] - old_time);
				else frames[index_fps] = 2000;

				index_fps++;
				index_fps &= 7;
				FPS = 0.0f;

				for(i = 0; i < 8; i++) FPS += frames[i];

				FPS /= 8.0f;
				old_time = new_time[0];
				view_fps = 0;
			}

			sprintf(Info_String, "%.1f", FPS);
		}
		else
		{
			QueryPerformanceFrequency((union _LARGE_INTEGER *) freq_cpu);
			if (freq_cpu[0] == 0) freq_cpu[0] = 1;

			sprintf(Info_String, "", FPS);
		}
		Print_Text(Info_String, strlen(Info_String), 10, 210, FPS_Style);
	}
}

GLbyte BlitData[1024*1024*3];

int Flip(HWND hWnd)
{
	if(!GLRC)
		return 0; // bail if directdraw hasn't been initialized yet or if we're still in the middle of initializing it

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	HRESULT rval = 1;
	RECT RectDest, RectSrc;
	int bpp = Bits32 ? 4 : 2; // Modif N. -- added: bytes per pixel

	DrawInformationOnTheScreen(); // Modif N. -- moved all this stuff out to its own function

	if (Fast_Blur) Half_Blur();
	
	CalculateDrawArea(hWnd, RectDest, RectSrc);

	int Src_X = (RectSrc.right - RectSrc.left);
	int Src_Y = (RectSrc.bottom - RectSrc.top);

	int Clr_Cmp_Val  = IS_FULL_X_RESOLUTION ? 4 : 8;
		Clr_Cmp_Val |= IS_FULL_Y_RESOLUTION ? 16 : 32;

	if (Flag_Clr_Scr & 0x100) // need clear second buffer
	{
		//Clear_Primary_Screen(hWnd);
		Flag_Clr_Scr ^= 0x300; // already cleared
	}

	if ((Flag_Clr_Scr & 0xFF) != (Clr_Cmp_Val & 0xFF))
	{
		/*if (!Full_Screen && W_VSync)
			lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
		if (!(Flag_Clr_Scr & 0x200))
			Clear_Primary_Screen(hWnd); // already cleared
		if ((!Full_Screen && Render_W >= 2)
		 || ( Full_Screen && Render_FS >= 2))
			Clear_Back_Screen(hWnd);
		*/
		if (Full_Screen && FS_VSync)
			Flag_Clr_Scr = Clr_Cmp_Val | 0x100; // need to clear second buffer
		else
			Flag_Clr_Scr = Clr_Cmp_Val;
	}

	Flag_Clr_Scr &= 0x1FF; // remove "already cleared"

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

	glViewport(0, 0, RectDest.right-RectDest.left, RectDest.bottom-RectDest.top);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslated(-1.0, 1.0, 1.0);
	glScaled(2.0/(RectDest.right-RectDest.left), -2.0/(RectDest.bottom-RectDest.top), 1.0);		

	glTranslated((RectDest.right-RectDest.left-(RectSrc.right-RectSrc.left))/2,
		(RectDest.bottom-RectDest.top-(RectSrc.bottom-RectSrc.top))/2, 0.0);

	glEnable(GL_TEXTURE_2D);
	//glActiveTexture(GL_TEXTURE0);

	glUseProgram(MainShader);
	glEnable(GL_BLEND);
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

	int w = RectDest.right-RectDest.left;
	int h = RectDest.bottom-RectDest.top;
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

	int ring_current = GetLong(&Ram_68k[0xEE42]);
	int ring_start = 0;
	int ring_end = 0;
	for (int i=ring_current; i>=0 && i > ring_current-4000; i-=4)
		if (GetLong(&Rom_Data[i]) == 0)
		{
			ring_start = i;
			break;
		}

	for (int i=ring_current; i<sizeof(Rom_Data) && i < ring_current+4000; i+=4)
		if (Rom_Data[(i)^1] == 0xFF
		 && Rom_Data[(i+1)^1] == 0xFF)
		 {
			ring_end = i;
			break;
		 }

	if (ring_start && ring_end
	&& ring_end - ring_start < 512*4)
	{
		int camerax = GetWord(&Ram_68k[0xEE78]);
		int cameray = GetWord(&Ram_68k[0xEE7C]);

		glUseProgram(SpriteShader);
		GLint res = glGetUniformLocation(SpriteShader, "sprite_entry");
		if (res != -1)
		{
			switch (Ram_68k[0xFEB3^1])
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
		res = glGetUniformLocation(SpriteShader, "PAL");
		if (res != -1)
		{
			glUniform1i(res, 1); //Texture unit 0 is for base images.
		}
		res = glGetUniformLocation(SpriteShader, "VRAM");
		if (res != -1)
		{
			glUniform1i(res, 2); //Texture unit 0 is for base images.
		}
		

		glBegin(GL_TRIANGLES);
		for (int i=0; ring_start+i<ring_end; i+=4)
		{
			if (GetWord(&Ram_68k[0xE700+(i>>1)]) != 0)
				continue;
			float rx = GetWord(&Rom_Data[ring_start+i]) - camerax;
			float ry = GetWord(&Rom_Data[ring_start+i+2]) - cameray;
			float rw = 8;
			if (Ram_68k[0xFEB3^1] ==  2)
				rw = 4;
			float rh = 8;
			//if (rx < 0 || rx > w)
			//	continue;
			//if (ry < 0 || ry > h)
			//	continue;

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
		}
		glEnd();

		glUseProgram(0);

		glUseProgram(SpriteShader);
		int link = 0;
		for (int i=0; i<80; ++i)
		{
			int so = 0xF800+link*8;
			int size = VRam[(so+2)^1];
			GLint res = glGetUniformLocation(SpriteShader, "sprite_entry");
			if (res != -1)
			{
				glUniform1i(res, GetWord(&VRam[so+4])|((size&0xF)<<16)); //Texture unit 0 is for base images.
			}

			int yy = GetWord(&VRam[so]);
			if (yy == 0)
				continue;
			int xx = GetWord(&VRam[so+6]);
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
			link = VRam[(so+3)^1];
			if (link == 0)
				break;
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

	/*if (Full_Screen)
	{
		if (Render_FS < 2)
		{
			if (FS_VSync)
			{
				lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
				lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
			}
			else
			{
				lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
//				lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
			}
		}
		else
		{
			// save Primary or Flip surface
			LPDIRECTDRAWSURFACE4 curBlit = lpDDS_Blit;

			// check equal size
			if ((RectDest.right - RectDest.left) != Src_X
			 || (RectDest.bottom - RectDest.top) != Src_Y)
				curBlit = lpDDS_Back; // replace with Back

			rval = curBlit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

			if (FAILED(rval)) goto cleanup_flip;

			if (curBlit == lpDDS_Back) // note: this can happen in windowed fullscreen, or if Correct_256_Aspect_Ratio is defined and the current display mode is 256 pixels across
			{
				// blit into lpDDS_Back first
				Blit_FS((unsigned char *) ddsd.lpSurface + ddsd.lPitch * RectSrc.top + RectSrc.left * bpp, ddsd.lPitch, Src_X / 2, Src_Y / 2, (16 + 320 - Src_X / 2) * bpp);

				curBlit->Unlock(NULL);

				if (FS_VSync)
				{
					lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
				else
					// blit into Primary
					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
			}
			else
			{
				// blit direct on screen (or flip if VSync)
				Blit_FS((unsigned char *) ddsd.lpSurface + ddsd.lPitch * RectDest.top + RectDest.left * bpp, ddsd.lPitch, Src_X / 2, Src_Y / 2, (16 + 320 - Src_X / 2) * bpp);

				curBlit->Unlock(NULL);

				if (FS_VSync)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
		}
	}
	else
	{
		// Window
		lpDDS_Blit = lpDDS_Back;

		if (Render_W >= 2)
		{
			rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

			if (FAILED(rval)) goto cleanup_flip;

			Blit_W((unsigned char *) ddsd.lpSurface + ddsd.lPitch * RectSrc.top + RectSrc.left * bpp, ddsd.lPitch, Src_X / 2, Src_Y / 2, (16 + 320 - Src_X / 2) * bpp);

			lpDDS_Blit->Unlock(NULL);
		}

		if (RectDest.top < RectDest.bottom)
		{
			if (W_VSync)
			{
				// wait if it is clearing primary, wait for VBlank only after this
				while (lpDDS_Primary->GetBltStatus(DDGBS_ISBLTDONE) == DDERR_WASSTILLDRAWING);
				/*int vb;
				lpDD->GetVerticalBlankStatus(&vb);
				if (!vb)*/// lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
//			}

		//	rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
//			rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
//		}
//	}

//cleanup_flip:
//	if (rval == DDERR_SURFACELOST)
//		rval = RestoreGraphics(hWnd);

	return 1;
}


int Update_Gens_Logo(HWND hWnd)
{
	int i, j, m, n;
	static short tab[64000], Init = 0;
	static float renv = 0, ang = 0, zoom_x = 0, zoom_y = 0, pas;
	unsigned short c;

	if (!Init)
	{
		HBITMAP Logo;

		Logo = LoadBitmap(ghInstance,  MAKEINTRESOURCE(IDB_LOGO_BIG));
		GetBitmapBits(Logo, 64000 * 2, tab);
		pas = 0.05f;
		Init = 1;
	}

	renv += pas;
	zoom_x = sin(renv);
	if (zoom_x == 0.0f) zoom_x = 0.0000001f;
	zoom_x = (1 / zoom_x) * 1;
	zoom_y = 1;

	if (IS_FULL_X_RESOLUTION)
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 320; i++)
			{
				m = (int)((float)(i - 160) * zoom_x);
				n = (int)((float)(j - 120) * zoom_y);

				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}
	else
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 256; i++)
			{
				m = (int)((float)(i - 128) * zoom_x);
				n = (int)((float)(j - 120) * zoom_y);

				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}

	Half_Blur();
	Flip(hWnd);

	return 1;
}


int Update_Crazy_Effect(HWND hWnd)
{
	int i, j, offset;
	int r = 0, v = 0, b = 0, prev_l, prev_p;
	int RB, G;

 	for(offset = 336 * 240, j = 0; j < 240; j++)
	{
		for(i = 0; i < 336; i++, offset--)
		{
			prev_l = MD_Screen32[offset - 336];
			prev_p = MD_Screen32[offset - 1];

			RB = ((prev_l & 0xFF00FF) + (prev_p & 0xFF00FF)) >> 1;
			G = ((prev_l & 0x00FF00) + (prev_p & 0x00FF00)) >> 1;

			if (Effect_Color & 0x4)
			{
				r = RB & 0xFF0000;
				if (rand() > 0x1600) r += 0x010000;
				else r -= 0x010000;
				if (r > 0xFF0000) r = 0xFF0000;
				else if (r < 0x00FFFF) r = 0;
			}

			if (Effect_Color & 0x2)
			{
				v = G & 0x00FF00;
				if (rand() > 0x1600) v += 0x000100;
				else v -= 0x000100;
				if (v > 0xFF00) v = 0xFF00;
				else if (v < 0x00FF) v = 0;
			}

			if (Effect_Color & 0x1)
			{
				b = RB & 0xFF;
				if (rand() > 0x1600) b++;
				else b--;
				if (b > 0xFF) b = 0xFF;
				else if (b < 0) b = 0;
			}
			MD_Screen32[offset] = r + v + b;
			prev_l = MD_Screen[offset - 336];
			prev_p = MD_Screen[offset - 1];

			if (Mode_555 & 1)
			{
				RB = ((prev_l & 0x7C1F) + (prev_p & 0x7C1F)) >> 1;
				G = ((prev_l & 0x03E0) + (prev_p & 0x03E0)) >> 1;

				if (Effect_Color & 0x4)
				{
					r = RB & 0x7C00;
					if (rand() > 0x2C00) r += 0x0400;
					else r -= 0x0400;
					if (r > 0x7C00) r = 0x7C00;
					else if (r < 0x0400) r = 0;
				}

				if (Effect_Color & 0x2)
				{
					v = G & 0x03E0;
					if (rand() > 0x2C00) v += 0x0020;
					else v -= 0x0020;
					if (v > 0x03E0) v = 0x03E0;
					else if (v < 0x0020) v = 0;
				}

				if (Effect_Color & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}
			else
			{
				RB = ((prev_l & 0xF81F) + (prev_p & 0xF81F)) >> 1;
				G = ((prev_l & 0x07C0) + (prev_p & 0x07C0)) >> 1;

				if (Effect_Color & 0x4)
				{
					r = RB & 0xF800;
					if (rand() > 0x2C00) r += 0x0800;
					else r -= 0x0800;
					if (r > 0xF800) r = 0xF800;
					else if (r < 0x0800) r = 0;
				}

				if (Effect_Color & 0x2)
				{
					v = G & 0x07C0;
					if (rand() > 0x2C00) v += 0x0040;
					else v -= 0x0040;
					if (v > 0x07C0) v = 0x07C0;
					else if (v < 0x0040) v = 0;
				}

				if (Effect_Color & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}

			MD_Screen[offset] = r + v + b;
		}
	}

	Flip(hWnd);

	return 1;
}

void UpdateInput()
{
	if(MainMovie.Status==MOVIE_PLAYING)
		MoviePlayingStuff();
	else
		Update_Controllers();

	//// XXX - probably most people won't need to use this?
	////Modif N - playback one player while recording another player
	//if(MainMovie.Status==MOVIE_RECORDING)
	//{
	//	if(GetKeyState(VK_CAPITAL) != 0)
	//	{
	//		extern void MoviePlayPlayer2();
	//		MoviePlayPlayer2();
	//	}
	//	if(GetKeyState(VK_NUMLOCK) != 0)
	//	{
	//		extern void MoviePlayPlayer1();
	//		MoviePlayPlayer1();
	//	}
	//}
}

void UpdateLagCount()
{
	if (Lag_Frame)
	{
		LagCount++;
		LagCountPersistent++;
	}

	// catch-all to fix problem with sound stuttered when paused during frame skipping
	// looks out of place but maybe this function should be renamed
	extern bool soundCleared;
	soundCleared = false;
}

int Dont_Skip_Next_Frame = 0;
void Prevent_Next_Frame_Skipping()
{
	Dont_Skip_Next_Frame = 8;
}

int Update_Emulation(HWND hWnd)
{
	int prevFrameCount = FrameCount;

	static int Over_Time = 0;
	int current_div;

	int Temp_Frame_Skip = Frame_Skip; //Modif N - part of a quick hack to make Tab the fast-forward key
	if (TurboMode)
		Temp_Frame_Skip = 8;
	if (SeekFrame) // If we've set a frame to seek to
	{
		if(FrameCount < (SeekFrame - 5000))      // we fast forward
			Temp_Frame_Skip = 10;
		else if (FrameCount < (SeekFrame - 100)) // at a variable rate
			Temp_Frame_Skip = 0;
		else 						     // based on distance to target
			Temp_Frame_Skip = -1;
		if(((SeekFrame - FrameCount) == 1))
		{
			Paused = 1; //then pause when we arrive
			SeekFrame = 0; //and clear the seek target
			Put_Info("Seek complete. Paused");
			Clear_Sound_Buffer();
			MustUpdateMenu = 1;
		}
	}
	if (Temp_Frame_Skip != -1)
	{
		if (AVIRecording && CleanAvi && (AVIWaitMovie == 0 || MainMovie.Status==MOVIE_PLAYING || MainMovie.Status==MOVIE_FINISHED))
			Temp_Frame_Skip = 0;
		if (Sound_Enable)
		{
			WP = (WP + 1) & (Sound_Segs - 1);

			if (WP == Get_Current_Seg())
				WP = (WP + Sound_Segs - 1) & (Sound_Segs - 1);

			Write_Sound_Buffer(NULL);
		}
		if (!CleanAvi) Take_Shot_AVI(hWnd);
		UpdateInput(); //Modif N
		if(MainMovie.Status==MOVIE_RECORDING)	//Modif
			MovieRecordingStuff();
		FrameCount++; //Modif

		if (Frame_Number++ < Temp_Frame_Skip) //Modif N - part of a quick hack to make Tab the fast-forward key
		{
			Lag_Frame = 1;
			Update_Frame_Fast_Hook();
			Update_RAM_Cheats();
			UpdateLagCount();
		}
		else
		{
			Frame_Number = 0;
			Lag_Frame = 1;
			Update_Frame_Hook();
			Update_RAM_Cheats();
			UpdateLagCount();
			if (CleanAvi) Take_Shot_AVI(hWnd);
			Flip(hWnd);
		}
	}
	else
	{
		if (Sound_Enable)
		{
			while (WP == Get_Current_Seg()) Sleep(Sleep_Time);
			RP = Get_Current_Seg();
			while (WP != RP)
			{
				Write_Sound_Buffer(NULL);
				WP = (WP + 1) & (Sound_Segs - 1);
				if(SlowDownMode)
				{
					if(SlowFrame)
						SlowFrame--;
					else
					{
						SlowFrame=SlowDownSpeed;
						UpdateInput(); //Modif N
						if (!CleanAvi) Take_Shot_AVI(hWnd);
						if(MainMovie.Status==MOVIE_RECORDING)	//Modif
							MovieRecordingStuff();
						FrameCount++; //Modif

						// note: we check for RamSearchHWnd because if it's open then it's likely causing most of any slowdown we get, in which case skipping renders will only make the slowdown appear worse
						if (WP != RP && AVIRecording==0 && Never_Skip_Frame==0 && !Dont_Skip_Next_Frame && !(RamSearchHWnd || RamWatchHWnd))
						{
							Lag_Frame = 1;
							Update_Frame_Fast_Hook();
							Update_RAM_Cheats();
							UpdateLagCount();
						}
						else
						{
							Lag_Frame = 1;
							Update_Frame_Hook();
							Update_RAM_Cheats();
							UpdateLagCount();
							if (CleanAvi) Take_Shot_AVI(hWnd);
							Flip(hWnd);
						}
					}
				}
				else
				{
					if (!CleanAvi) Take_Shot_AVI(hWnd);
					UpdateInput(); //Modif N
					if(MainMovie.Status==MOVIE_RECORDING)	//Modif
						MovieRecordingStuff();
					FrameCount++; //Modif

					if (WP != RP && AVIRecording==0 && Never_Skip_Frame==0 && !Dont_Skip_Next_Frame && !(RamSearchHWnd || RamWatchHWnd))
					{
						Lag_Frame = 1;
						Update_Frame_Fast_Hook();
						Update_RAM_Cheats();
						UpdateLagCount();
					}
					else
					{
						Lag_Frame = 1;
						Update_Frame_Hook();
						Update_RAM_Cheats();
						UpdateLagCount();
						if (CleanAvi) Take_Shot_AVI(hWnd);
						Flip(hWnd);
					}
				}
				if (Never_Skip_Frame || Dont_Skip_Next_Frame)
					WP=RP;
			}
		}
		else
		{
			if (CPU_Mode) current_div = 20;
			else current_div = 16 + (Over_Time ^= 1);

			if(SlowDownMode)
				current_div*=(SlowDownSpeed+1);

			New_Time = GetTickCount();
			Used_Time += (New_Time - Last_Time);
			Frame_Number = Used_Time / current_div;
			Used_Time %= current_div;
			Last_Time = New_Time;
			
			if (Frame_Number > 8) Frame_Number = 8;
			if((Never_Skip_Frame!=0 || Dont_Skip_Next_Frame) && Frame_Number>0)
				Frame_Number=1;
			/*if(SlowDownMode)
			{
				if(SlowFrame)
				{
					Frame_Number>>=1; //Modif
					SlowFrame--;
				}
				else
					SlowFrame=SlowDownSpeed;
			}*/

			for (; Frame_Number > 1; Frame_Number--)
			{
				if (!CleanAvi) Take_Shot_AVI(hWnd);
				UpdateInput(); //Modif N
				if(MainMovie.Status==MOVIE_RECORDING)	//Modif
					MovieRecordingStuff();
				FrameCount++; //Modif

				if(AVIRecording==0 && Never_Skip_Frame==0 && !Dont_Skip_Next_Frame)
				{
					Lag_Frame = 1;
					Update_Frame_Fast_Hook();
					Update_RAM_Cheats();
					UpdateLagCount();
				
				}
				else
				{
					Lag_Frame = 1;
					Update_Frame_Hook();
					Update_RAM_Cheats();
					UpdateLagCount();
					if (CleanAvi) Take_Shot_AVI(hWnd);
					Flip(hWnd);
				}
			}

			if (Frame_Number)
			{
				if (!CleanAvi) Take_Shot_AVI(hWnd);
				UpdateInput(); //Modif N
				if(MainMovie.Status==MOVIE_RECORDING)	//Modif
					MovieRecordingStuff();
				FrameCount++; //Modif
				Lag_Frame = 1;
				Update_Frame_Hook();
				Update_RAM_Cheats();
				UpdateLagCount();
				if (CleanAvi) Take_Shot_AVI(hWnd);
				Flip(hWnd);
			}
			else Sleep(Sleep_Time);
		}
	}

	if(Dont_Skip_Next_Frame)
		Dont_Skip_Next_Frame--;

	return prevFrameCount != FrameCount;
}

void Update_Emulation_One_Before(HWND hWnd)
{
	if (Sound_Enable)
	{
		WP = (WP + 1) & (Sound_Segs - 1);

		if (WP == Get_Current_Seg())
			WP = (WP + Sound_Segs - 1) & (Sound_Segs - 1);

		Write_Sound_Buffer(NULL);
	}
	UpdateInput(); //Modif N
	if(MainMovie.Status==MOVIE_RECORDING)	//Modif
		MovieRecordingStuff();
	if (!CleanAvi) Take_Shot_AVI(hWnd);
	FrameCount++; //Modif
	Lag_Frame = 1;
}

void Update_Emulation_One_Before_Minimal()
{
	UpdateInput();
	if(MainMovie.Status==MOVIE_RECORDING)
		MovieRecordingStuff();
	FrameCount++;
	Lag_Frame = 1;
}

void Update_Emulation_One_After(HWND hWnd)
{
	Update_RAM_Cheats();
	UpdateLagCount();
	if (CleanAvi) Take_Shot_AVI(hWnd);
	Flip(hWnd);
}

void Update_Emulation_After_Fast(HWND hWnd)
{
	Update_RAM_Cheats();
	UpdateLagCount();

	int Temp_Frame_Skip = Frame_Skip;
	if (TurboMode)
		Temp_Frame_Skip = 8;
	if (AVIRecording && CleanAvi && (AVIWaitMovie == 0 || MainMovie.Status==MOVIE_PLAYING || MainMovie.Status==MOVIE_FINISHED))
		Temp_Frame_Skip = 0;
	if (Frame_Number > 8) Frame_Number = 8;
	if (Frame_Number++ < Temp_Frame_Skip)
		return;
	Frame_Number = 0;

	if (CleanAvi) Take_Shot_AVI(hWnd);
	Flip(hWnd);
}

void Update_Emulation_After_Controlled(HWND hWnd, bool flip)
{
	Update_RAM_Cheats();
	UpdateLagCount();

	if(flip)
	{
		if (CleanAvi) Take_Shot_AVI(hWnd);
		Flip(hWnd);
	}
}

int Update_Emulation_One(HWND hWnd)
{
	Update_Emulation_One_Before(hWnd);
	Update_Frame_Hook();
	Update_Emulation_One_After(hWnd);
	return 1;
}

int Update_Emulation_Netplay(HWND hWnd, int player, int num_player)
{
	static int Over_Time = 0;
	int current_div;

	if (CPU_Mode) current_div = 20;
	else current_div = 16 + (Over_Time ^= 1);

	New_Time = GetTickCount();
	Used_Time += (New_Time - Last_Time);
	Frame_Number = Used_Time / current_div;
	Used_Time %= current_div;
	Last_Time = New_Time;

	if (Frame_Number > 6) Frame_Number = 6;

	for (; Frame_Number > 1; Frame_Number--)
	{
		if (Sound_Enable)
		{
			if (WP == Get_Current_Seg()) WP = (WP - 1) & (Sound_Segs - 1);
			Write_Sound_Buffer(NULL);
			WP = (WP + 1) & (Sound_Segs - 1);
		}

		Scan_Player_Net(player);
		if (Kaillera_Error != -1) Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		//Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		if(MainMovie.Status==MOVIE_PLAYING)
			MoviePlayingStuff();
		else
			Update_Controllers_Net(num_player);
		if(MainMovie.Status==MOVIE_RECORDING)	//Modif
			MovieRecordingStuff();
		FrameCount++;
		Lag_Frame = 1;
		Update_Frame_Fast_Hook();
		Update_RAM_Cheats();
		UpdateLagCount();
	}

	if (Frame_Number)
	{
		if (Sound_Enable)
		{
			if (WP == Get_Current_Seg()) WP = (WP - 1) & (Sound_Segs - 1);
			Write_Sound_Buffer(NULL);
			WP = (WP + 1) & (Sound_Segs - 1);
		}

		Scan_Player_Net(player);
		if (Kaillera_Error != -1) Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		//Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		if(MainMovie.Status==MOVIE_PLAYING)
			MoviePlayingStuff();
		else
			Update_Controllers_Net(num_player);
		if(MainMovie.Status==MOVIE_RECORDING)	//Modif
			MovieRecordingStuff();
		FrameCount++;
		Lag_Frame = 1;
		Update_Frame_Hook();
		Update_RAM_Cheats();
		UpdateLagCount();
	
		Flip(hWnd);
	}

	return 1;
}

int Eff_Screen(void)
{
	int i;

	for(i = 0; i < 336 * 240; i++)
	{
		MD_Screen[i] = 0;
		MD_Screen32[i] = 0;
	}

	return 1;
}

int Pause_Screen(void)
{
	int i, j, offset;
	int r, v, b, nr, nv, nb;

	if(Disable_Blue_Screen) return 1;

	r = v = b = nr = nv = nb = 0;

	for(offset = j = 0; j < 240; j++)
	{
		for(i = 0; i < 336; i++, offset++)
		{
			r = (MD_Screen32[offset] & 0xFF0000) >> 16;
			v = (MD_Screen32[offset] & 0x00FF00) >> 8;
			b = (MD_Screen32[offset] & 0x0000FF);

			nr = nv = nb = (r + v + b) / 3;
			
			if ((nb <<= 1) > 255) nb = 255;

			nr &= 0xFE;
			nv &= 0xFE;
			nb &= 0xFE;

			MD_Screen32[offset] = (nr << 16) + (nv << 8) + nb;

			if (Mode_555 & 1)
			{
				r = (MD_Screen[offset] & 0x7C00) >> 10;
				v = (MD_Screen[offset] & 0x03E0) >> 5;
				b = (MD_Screen[offset] & 0x001F);
			}
			else
			{
				r = (MD_Screen[offset] & 0xF800) >> 11;
				v = (MD_Screen[offset] & 0x07C0) >> 6;
				b = (MD_Screen[offset] & 0x001F);
			}

			nr = nv = nb = (r + v + b) / 3;
			
			if ((nb <<= 1) > 31) nb = 31;

			nr &= 0x1E;
			nv &= 0x1E;
			nb &= 0x1E;

			if (Mode_555 & 1)
				MD_Screen[offset] = (nr << 10) + (nv << 5) + nb;
			else
				MD_Screen[offset] = (nr << 11) + (nv << 6) + nb;
		}
	}

	return 1;
}

int Show_Genesis_Screen(HWND hWnd)
{
	Do_VDP_Refresh();
	Flip(hWnd);

	return 1;
}
int Show_Genesis_Screen()
{
	return Show_Genesis_Screen(HWnd);
}

int Take_Shot()
{
	return Save_Shot(Bits32?(unsigned char*)MD_Screen32:(unsigned char*)MD_Screen,(Mode_555 & 1) | (Bits32?2:0),IS_FULL_X_RESOLUTION,IS_FULL_Y_RESOLUTION);
}

int Take_Shot_Clipboard()
{
	return Save_Shot_Clipboard(Bits32?(unsigned char*)MD_Screen32:(unsigned char*)MD_Screen,(Mode_555 & 1)|(Bits32?2:0),IS_FULL_X_RESOLUTION,IS_FULL_Y_RESOLUTION);
}

int Take_Shot_AVI(HWND hWnd)
{
	if(AVIRecording!=0 && (AVIWaitMovie==0 || MainMovie.Status==MOVIE_PLAYING || MainMovie.Status==MOVIE_FINISHED))
		return Save_Shot_AVI(Bits32?(unsigned char*)MD_Screen32:(unsigned char*)MD_Screen,(Mode_555&1)|(Bits32?2:0),IS_FULL_X_RESOLUTION,IS_FULL_Y_RESOLUTION,hWnd);
	else
		return 0;
}

/*
void MP3_init_test()
{
	FILE *f;
	f = fopen("\\vc\\gens\\release\\test.mp3", "rb");
	if (f == NULL) return;
	MP3_Test(f);
	Play_Sound();
}

void MP3_update_test()
{
	int *buf[2];
	buf[0] = Seg_L;
	buf[1] = Seg_R;
	while (WP == Get_Current_Seg());
	RP = Get_Current_Seg();
	while (WP != RP)
	{
		Write_Sound_Buffer(NULL);
		WP = (WP + 1) & (Sound_Segs - 1);
		memset(Seg_L, 0, (Seg_Length << 2));
		memset(Seg_R, 0, (Seg_Length << 2));
		MP3_Update(buf, Seg_Length);
	}
}
*/
