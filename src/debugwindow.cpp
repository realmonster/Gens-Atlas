// by r57shell

#include "resource.h"
#include "gens.h"
#include "save.h"
#include "g_main.h"
#include "ramwatch.h"
#include "debugwindow.h"
#include "G_ddraw.h"
#include <vector>

void Handle_Gens_Messages();
LRESULT CALLBACK EditBreakProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern int Gens_Running;
extern "C" int Clear_Sound_Buffer(void);

char * MsgName[]={
"WM_NULL",
"WM_CREATE",
"WM_DESTROY",
"WM_MOVE",
"0x0004",
"WM_SIZE",
"WM_ACTIVATE",
"WM_SETFOCUS",
"WM_KILLFOCUS",
"0x0009",
"WM_ENABLE",
"WM_SETREDRAW",
"WM_SETTEXT",
"WM_GETTEXT",
"WM_GETTEXTLENGTH",
"WM_PAINT",
"WM_CLOSE",
"WM_QUERYENDSESSION",
"WM_QUIT",
"WM_QUERYOPEN",
"WM_ERASEBKGND",
"WM_SYSCOLORCHANGE",
"WM_ENDSESSION",
"0x0017",
"WM_SHOWWINDOW",
"0x0019",
"WM_WININICHANGE",
"WM_DEVMODECHANGE",
"WM_ACTIVATEAPP",
"WM_FONTCHANGE",
"WM_TIMECHANGE",
"WM_CANCELMODE",
"WM_SETCURSOR",
"WM_MOUSEACTIVATE",
"WM_CHILDACTIVATE",
"WM_QUEUESYNC",
"WM_GETMINMAXINFO",
"0x0025",
"WM_PAINTICON",
"WM_ICONERASEBKGND",
"WM_NEXTDLGCTL",
"0x0029",
"WM_SPOOLERSTATUS",
"WM_DRAWITEM",
"WM_MEASUREITEM",
"WM_DELETEITEM",
"WM_VKEYTOITEM",
"WM_CHARTOITEM",
"WM_SETFONT",
"WM_GETFONT",
"WM_SETHOTKEY",
"WM_GETHOTKEY",
"0x0034",
"0x0035",
"0x0036",
"WM_QUERYDRAGICON",
"0x0038",
"WM_COMPAREITEM",
"0x003A",
"0x003B",
"0x003C",
"WM_GETOBJECT",
"0x003E",
"0x003F",
"0x0040",
"WM_COMPACTING",
"0x0042",
"0x0043",
"WM_COMMNOTIFY",
"0x0045",
"WM_WINDOWPOSCHANGING",
"WM_WINDOWPOSCHANGED",
"WM_POWER",
"0x0049",
"WM_COPYDATA",
"WM_CANCELJOURNAL",
"0x004C",
"0x004D",
"WM_NOTIFY",
"0x004F",
"WM_INPUTLANGCHANGEREQUEST",
"WM_INPUTLANGCHANGE",
"WM_TCARD",
"WM_HELP",
"WM_USERCHANGED",
"WM_NOTIFYFORMAT",
"0x0056",
"0x0057",
"0x0058",
"0x0059",
"0x005A",
"0x005B",
"0x005C",
"0x005D",
"0x005E",
"0x005F",
"0x0060",
"0x0061",
"0x0062",
"0x0063",
"0x0064",
"0x0065",
"0x0066",
"0x0067",
"0x0068",
"0x0069",
"0x006A",
"0x006B",
"0x006C",
"0x006D",
"0x006E",
"0x006F",
"0x0070",
"0x0071",
"0x0072",
"0x0073",
"0x0074",
"0x0075",
"0x0076",
"0x0077",
"0x0078",
"0x0079",
"0x007A",
"WM_CONTEXTMENU",
"WM_STYLECHANGING",
"WM_STYLECHANGED",
"WM_DISPLAYCHANGE",
"WM_GETICON",
"WM_SETICON",
"WM_NCCREATE",
"WM_NCDESTROY",
"WM_NCCALCSIZE",
"WM_NCHITTEST",
"WM_NCPAINT",
"WM_NCACTIVATE",
"WM_GETDLGCODE",
"WM_SYNCPAINT",
"0x0089",
"0x008A",
"0x008B",
"0x008C",
"0x008D",
"0x008E",
"0x008F",
"0x0090",
"0x0091",
"0x0092",
"0x0093",
"0x0094",
"0x0095",
"0x0096",
"0x0097",
"0x0098",
"0x0099",
"0x009A",
"0x009B",
"0x009C",
"0x009D",
"0x009E",
"0x009F",
"WM_NCMOUSEMOVE",
"WM_NCLBUTTONDOWN",
"WM_NCLBUTTONUP",
"WM_NCLBUTTONDBLCLK",
"WM_NCRBUTTONDOWN",
"WM_NCRBUTTONUP",
"WM_NCRBUTTONDBLCLK",
"WM_NCMBUTTONDOWN",
"WM_NCMBUTTONUP",
"WM_NCMBUTTONDBLCLK",
"0x00AA",
"WM_NCXBUTTONDOWN",
"WM_NCXBUTTONUP",
"WM_NCXBUTTONDBLCLK",
"0x00AE",
"0x00AF",
"0x00B0",
"0x00B1",
"0x00B2",
"0x00B3",
"0x00B4",
"0x00B5",
"0x00B6",
"0x00B7",
"0x00B8",
"0x00B9",
"0x00BA",
"0x00BB",
"0x00BC",
"0x00BD",
"0x00BE",
"0x00BF",
"0x00C0",
"0x00C1",
"0x00C2",
"0x00C3",
"0x00C4",
"0x00C5",
"0x00C6",
"0x00C7",
"0x00C8",
"0x00C9",
"0x00CA",
"0x00CB",
"0x00CC",
"0x00CD",
"0x00CE",
"0x00CF",
"0x00D0",
"0x00D1",
"0x00D2",
"0x00D3",
"0x00D4",
"0x00D5",
"0x00D6",
"0x00D7",
"0x00D8",
"0x00D9",
"0x00DA",
"0x00DB",
"0x00DC",
"0x00DD",
"0x00DE",
"0x00DF",
"0x00E0",
"0x00E1",
"0x00E2",
"0x00E3",
"0x00E4",
"0x00E5",
"0x00E6",
"0x00E7",
"0x00E8",
"0x00E9",
"0x00EA",
"0x00EB",
"0x00EC",
"0x00ED",
"0x00EE",
"0x00EF",
"0x00F0",
"0x00F1",
"0x00F2",
"0x00F3",
"0x00F4",
"0x00F5",
"0x00F6",
"0x00F7",
"0x00F8",
"0x00F9",
"0x00FA",
"0x00FB",
"0x00FC",
"0x00FD",
"0x00FE",
"WM_INPUT",
"WM_KEYDOWN",
"WM_KEYUP",
"WM_CHAR",
"WM_DEADCHAR",
"WM_SYSKEYDOWN",
"WM_SYSKEYUP",
"WM_SYSCHAR",
"WM_SYSDEADCHAR",
"WM_KEYLAST",
"WM_KEYLAST",
"0x010A",
"0x010B",
"0x010C",
"WM_IME_STARTCOMPOSITION",
"WM_IME_ENDCOMPOSITION",
"WM_IME_KEYLAST",
"WM_INITDIALOG",
"WM_COMMAND",
"WM_SYSCOMMAND",
"WM_TIMER",
"WM_HSCROLL",
"WM_VSCROLL",
"WM_INITMENU",
"WM_INITMENUPOPUP",
"0x0118",
"0x0119",
"0x011A",
"0x011B",
"0x011C",
"0x011D",
"0x011E",
"WM_MENUSELECT",
"WM_MENUCHAR",
"WM_ENTERIDLE",
"WM_MENURBUTTONUP",
"WM_MENUDRAG",
"WM_MENUGETOBJECT",
"WM_UNINITMENUPOPUP",
"WM_MENUCOMMAND",
"WM_CHANGEUISTATE",
"WM_UPDATEUISTATE",
"WM_QUERYUISTATE",
"0x012A",
"0x012B",
"0x012C",
"0x012D",
"0x012E",
"0x012F",
"0x0130",
"0x0131",
"WM_CTLCOLORMSGBOX",
"WM_CTLCOLOREDIT",
"WM_CTLCOLORLISTBOX",
"WM_CTLCOLORBTN",
"WM_CTLCOLORDLG",
"WM_CTLCOLORSCROLLBAR",
"WM_CTLCOLORSTATIC",
"0x0139",
"0x013A",
"0x013B",
"0x013C",
"0x013D",
"0x013E",
"0x013F",
"0x0140",
"0x0141",
"0x0142",
"0x0143",
"0x0144",
"0x0145",
"0x0146",
"0x0147",
"0x0148",
"0x0149",
"0x014A",
"0x014B",
"0x014C",
"0x014D",
"0x014E",
"0x014F",
"0x0150",
"0x0151",
"0x0152",
"0x0153",
"0x0154",
"0x0155",
"0x0156",
"0x0157",
"0x0158",
"0x0159",
"0x015A",
"0x015B",
"0x015C",
"0x015D",
"0x015E",
"0x015F",
"0x0160",
"0x0161",
"0x0162",
"0x0163",
"0x0164",
"0x0165",
"0x0166",
"0x0167",
"0x0168",
"0x0169",
"0x016A",
"0x016B",
"0x016C",
"0x016D",
"0x016E",
"0x016F",
"0x0170",
"0x0171",
"0x0172",
"0x0173",
"0x0174",
"0x0175",
"0x0176",
"0x0177",
"0x0178",
"0x0179",
"0x017A",
"0x017B",
"0x017C",
"0x017D",
"0x017E",
"0x017F",
"0x0180",
"0x0181",
"0x0182",
"0x0183",
"0x0184",
"0x0185",
"0x0186",
"0x0187",
"0x0188",
"0x0189",
"0x018A",
"0x018B",
"0x018C",
"0x018D",
"0x018E",
"0x018F",
"0x0190",
"0x0191",
"0x0192",
"0x0193",
"0x0194",
"0x0195",
"0x0196",
"0x0197",
"0x0198",
"0x0199",
"0x019A",
"0x019B",
"0x019C",
"0x019D",
"0x019E",
"0x019F",
"0x01A0",
"0x01A1",
"0x01A2",
"0x01A3",
"0x01A4",
"0x01A5",
"0x01A6",
"0x01A7",
"0x01A8",
"0x01A9",
"0x01AA",
"0x01AB",
"0x01AC",
"0x01AD",
"0x01AE",
"0x01AF",
"0x01B0",
"0x01B1",
"0x01B2",
"0x01B3",
"0x01B4",
"0x01B5",
"0x01B6",
"0x01B7",
"0x01B8",
"0x01B9",
"0x01BA",
"0x01BB",
"0x01BC",
"0x01BD",
"0x01BE",
"0x01BF",
"0x01C0",
"0x01C1",
"0x01C2",
"0x01C3",
"0x01C4",
"0x01C5",
"0x01C6",
"0x01C7",
"0x01C8",
"0x01C9",
"0x01CA",
"0x01CB",
"0x01CC",
"0x01CD",
"0x01CE",
"0x01CF",
"0x01D0",
"0x01D1",
"0x01D2",
"0x01D3",
"0x01D4",
"0x01D5",
"0x01D6",
"0x01D7",
"0x01D8",
"0x01D9",
"0x01DA",
"0x01DB",
"0x01DC",
"0x01DD",
"0x01DE",
"0x01DF",
"0x01E0",
"0x01E1",
"0x01E2",
"0x01E3",
"0x01E4",
"0x01E5",
"0x01E6",
"0x01E7",
"0x01E8",
"0x01E9",
"0x01EA",
"0x01EB",
"0x01EC",
"0x01ED",
"0x01EE",
"0x01EF",
"0x01F0",
"0x01F1",
"0x01F2",
"0x01F3",
"0x01F4",
"0x01F5",
"0x01F6",
"0x01F7",
"0x01F8",
"0x01F9",
"0x01FA",
"0x01FB",
"0x01FC",
"0x01FD",
"0x01FE",
"0x01FF",
"WM_MOUSEMOVE",
"WM_LBUTTONDOWN",
"WM_LBUTTONUP",
"WM_LBUTTONDBLCLK",
"WM_RBUTTONDOWN",
"WM_RBUTTONUP",
"WM_RBUTTONDBLCLK",
"WM_MBUTTONDOWN",
"WM_MBUTTONUP",
"WM_MOUSELAST",
"WM_MOUSELAST",
"WM_XBUTTONDOWN",
"WM_XBUTTONUP",
"WM_MOUSELAST",
"0x020E",
"0x020F",
"WM_PARENTNOTIFY",
"WM_ENTERMENULOOP",
"WM_EXITMENULOOP",
"WM_NEXTMENU",
"WM_SIZING",
"WM_CAPTURECHANGED",
"WM_MOVING",
"0x0217",
"WM_POWERBROADCAST",
"WM_DEVICECHANGE",
"0x021A",
"0x021B",
"0x021C",
"0x021D",
"0x021E",
"0x021F",
"WM_MDICREATE",
"WM_MDIDESTROY",
"WM_MDIACTIVATE",
"WM_MDIRESTORE",
"WM_MDINEXT",
"WM_MDIMAXIMIZE",
"WM_MDITILE",
"WM_MDICASCADE",
"WM_MDIICONARRANGE",
"WM_MDIGETACTIVE",
"0x022A",
"0x022B",
"0x022C",
"0x022D",
"0x022E",
"0x022F",
"WM_MDISETMENU",
"WM_ENTERSIZEMOVE",
"WM_EXITSIZEMOVE",
"WM_DROPFILES",
"WM_MDIREFRESHMENU",
"0x0235",
"0x0236",
"0x0237",
"0x0238",
"0x0239",
"0x023A",
"0x023B",
"0x023C",
"0x023D",
"0x023E",
"0x023F",
"0x0240",
"0x0241",
"0x0242",
"0x0243",
"0x0244",
"0x0245",
"0x0246",
"0x0247",
"0x0248",
"0x0249",
"0x024A",
"0x024B",
"0x024C",
"0x024D",
"0x024E",
"0x024F",
"0x0250",
"0x0251",
"0x0252",
"0x0253",
"0x0254",
"0x0255",
"0x0256",
"0x0257",
"0x0258",
"0x0259",
"0x025A",
"0x025B",
"0x025C",
"0x025D",
"0x025E",
"0x025F",
"0x0260",
"0x0261",
"0x0262",
"0x0263",
"0x0264",
"0x0265",
"0x0266",
"0x0267",
"0x0268",
"0x0269",
"0x026A",
"0x026B",
"0x026C",
"0x026D",
"0x026E",
"0x026F",
"0x0270",
"0x0271",
"0x0272",
"0x0273",
"0x0274",
"0x0275",
"0x0276",
"0x0277",
"0x0278",
"0x0279",
"0x027A",
"0x027B",
"0x027C",
"0x027D",
"0x027E",
"0x027F",
"0x0280",
"WM_IME_SETCONTEXT",
"WM_IME_NOTIFY",
"WM_IME_CONTROL",
"WM_IME_COMPOSITIONFULL",
"WM_IME_SELECT",
"WM_IME_CHAR",
"0x0287",
"WM_IME_REQUEST",
"0x0289",
"0x028A",
"0x028B",
"0x028C",
"0x028D",
"0x028E",
"0x028F",
"WM_IME_KEYDOWN",
"WM_IME_KEYUP",
"0x0292",
"0x0293",
"0x0294",
"0x0295",
"0x0296",
"0x0297",
"0x0298",
"0x0299",
"0x029A",
"0x029B",
"0x029C",
"0x029D",
"0x029E",
"0x029F",
"WM_NCMOUSEHOVER",
"WM_MOUSEHOVER",
"WM_NCMOUSELEAVE",
"WM_MOUSELEAVE",
"0x02A4",
"0x02A5",
"0x02A6",
"0x02A7",
"0x02A8",
"0x02A9",
"0x02AA",
"0x02AB",
"0x02AC",
"0x02AD",
"0x02AE",
"0x02AF",
"0x02B0",
"WM_WTSSESSION_CHANGE",
"0x02B2",
"0x02B3",
"0x02B4",
"0x02B5",
"0x02B6",
"0x02B7",
"0x02B8",
"0x02B9",
"0x02BA",
"0x02BB",
"0x02BC",
"0x02BD",
"0x02BE",
"0x02BF",
"WM_TABLET_FIRST",
"0x02C1",
"0x02C2",
"0x02C3",
"0x02C4",
"0x02C5",
"0x02C6",
"0x02C7",
"0x02C8",
"0x02C9",
"0x02CA",
"0x02CB",
"0x02CC",
"0x02CD",
"0x02CE",
"0x02CF",
"0x02D0",
"0x02D1",
"0x02D2",
"0x02D3",
"0x02D4",
"0x02D5",
"0x02D6",
"0x02D7",
"0x02D8",
"0x02D9",
"0x02DA",
"0x02DB",
"0x02DC",
"0x02DD",
"0x02DE",
"WM_TABLET_LAST",
"0x02E0",
"0x02E1",
"0x02E2",
"0x02E3",
"0x02E4",
"0x02E5",
"0x02E6",
"0x02E7",
"0x02E8",
"0x02E9",
"0x02EA",
"0x02EB",
"0x02EC",
"0x02ED",
"0x02EE",
"0x02EF",
"0x02F0",
"0x02F1",
"0x02F2",
"0x02F3",
"0x02F4",
"0x02F5",
"0x02F6",
"0x02F7",
"0x02F8",
"0x02F9",
"0x02FA",
"0x02FB",
"0x02FC",
"0x02FD",
"0x02FE",
"0x02FF",
"WM_CUT",
"WM_COPY",
"WM_PASTE",
"WM_CLEAR",
"WM_UNDO",
"WM_RENDERFORMAT",
"WM_RENDERALLFORMATS",
"WM_DESTROYCLIPBOARD",
"WM_DRAWCLIPBOARD",
"WM_PAINTCLIPBOARD",
"WM_VSCROLLCLIPBOARD",
"WM_SIZECLIPBOARD",
"WM_ASKCBFORMATNAME",
"WM_CHANGECBCHAIN",
"WM_HSCROLLCLIPBOARD",
"WM_QUERYNEWPALETTE",
"WM_PALETTEISCHANGING",
"WM_PALETTECHANGED",
"WM_HOTKEY",
"0x0313",
"0x0314",
"0x0315",
"0x0316",
"WM_PRINT",
"WM_PRINTCLIENT",
"WM_APPCOMMAND",
"WM_THEMECHANGED",
"0x031B",
"0x031C",
"0x031D",
"0x031E",
"0x031F",
"0x0320",
"0x0321",
"0x0322",
"0x0323",
"0x0324",
"0x0325",
"0x0326",
"0x0327",
"0x0328",
"0x0329",
"0x032A",
"0x032B",
"0x032C",
"0x032D",
"0x032E",
"0x032F",
"0x0330",
"0x0331",
"0x0332",
"0x0333",
"0x0334",
"0x0335",
"0x0336",
"0x0337",
"0x0338",
"0x0339",
"0x033A",
"0x033B",
"0x033C",
"0x033D",
"0x033E",
"0x033F",
"0x0340",
"0x0341",
"0x0342",
"0x0343",
"0x0344",
"0x0345",
"0x0346",
"0x0347",
"0x0348",
"0x0349",
"0x034A",
"0x034B",
"0x034C",
"0x034D",
"0x034E",
"0x034F",
"0x0350",
"0x0351",
"0x0352",
"0x0353",
"0x0354",
"0x0355",
"0x0356",
"0x0357",
"WM_HANDHELDFIRST",
"0x0359",
"0x035A",
"0x035B",
"0x035C",
"0x035D",
"0x035E",
"WM_HANDHELDLAST",
"WM_AFXFIRST",
"0x0361",
"0x0362",
"0x0363",
"0x0364",
"0x0365",
"0x0366",
"0x0367",
"0x0368",
"0x0369",
"0x036A",
"0x036B",
"0x036C",
"0x036D",
"0x036E",
"0x036F",
"0x0370",
"0x0371",
"0x0372",
"0x0373",
"0x0374",
"0x0375",
"0x0376",
"0x0377",
"0x0378",
"0x0379",
"0x037A",
"0x037B",
"0x037C",
"0x037D",
"0x037E",
"WM_AFXLAST",
"WM_PENWINFIRST",
"0x0381",
"0x0382",
"0x0383",
"0x0384",
"0x0385",
"0x0386",
"0x0387",
"0x0388",
"0x0389",
"0x038A",
"0x038B",
"0x038C",
"0x038D",
"0x038E",
"WM_PENWINLAST",
"0x0390",
"0x0391",
"0x0392",
"0x0393",
"0x0394",
"0x0395",
"0x0396",
"0x0397",
"0x0398",
"0x0399",
"0x039A",
"0x039B",
"0x039C",
"0x039D",
"0x039E",
"0x039F",
"0x03A0",
"0x03A1",
"0x03A2",
"0x03A3",
"0x03A4",
"0x03A5",
"0x03A6",
"0x03A7",
"0x03A8",
"0x03A9",
"0x03AA",
"0x03AB",
"0x03AC",
"0x03AD",
"0x03AE",
"0x03AF",
"0x03B0",
"0x03B1",
"0x03B2",
"0x03B3",
"0x03B4",
"0x03B5",
"0x03B6",
"0x03B7",
"0x03B8",
"0x03B9",
"0x03BA",
"0x03BB",
"0x03BC",
"0x03BD",
"0x03BE",
"0x03BF",
"0x03C0",
"0x03C1",
"0x03C2",
"0x03C3",
"0x03C4",
"0x03C5",
"0x03C6",
"0x03C7",
"0x03C8",
"0x03C9",
"0x03CA",
"0x03CB",
"0x03CC",
"0x03CD",
"0x03CE",
"0x03CF",
"0x03D0",
"0x03D1",
"0x03D2",
"0x03D3",
"0x03D4",
"0x03D5",
"0x03D6",
"0x03D7",
"0x03D8",
"0x03D9",
"0x03DA",
"0x03DB",
"0x03DC",
"0x03DD",
"0x03DE",
"0x03DF",
"0x03E0",
"0x03E1",
"0x03E2",
"0x03E3",
"0x03E4",
"0x03E5",
"0x03E6",
"0x03E7",
"0x03E8",
"0x03E9",
"0x03EA",
"0x03EB",
"0x03EC",
"0x03ED",
"0x03EE",
"0x03EF",
"0x03F0",
"0x03F1",
"0x03F2",
"0x03F3",
"0x03F4",
"0x03F5",
"0x03F6",
"0x03F7",
"0x03F8",
"0x03F9",
"0x03FA",
"0x03FB",
"0x03FC",
"0x03FD",
"0x03FE",
"0x03FF",
"WM_USER"
};

char *GetMsgName(int x)
{
	if (x<0||x>sizeof(MsgName)/sizeof(MsgName[0]))
		return "UNKNOWN";
	return MsgName[x];
}

DebugWindow::DebugWindow()
{
	DummyHWnd=HWnd=NULL;
	DLGPROC DebugProc=NULL;
	memset(Fonts,0,sizeof(Fonts));
	LastBMP=NULL;
	MemBMP=NULL;
	MemDC=NULL;

	ScrollMin=0;
	ScrollMax=5;

	StepInto=false;
	IdaSync=false;
	StepOver=-1;
	SelectedLine=-1;
	Title="Debug";
	whyBreakpoint = "";
}

DebugWindow::~DebugWindow()
{
}

bool DebugWindow::IsShowedAddress(int pc)
{
	return false;
}

void DebugWindow::Update() {}
void DebugWindow::TracePC(int pc) {}
void DebugWindow::TraceRead(uint32 start,uint32 stop) {}
void DebugWindow::TraceWrite(uint32 start,uint32 stop) {}
int DebugWindow::DisasmLen(int pc) {return 1;}
void DebugWindow::DoStepOver() {}

int DebugWindow::GetNearestScroll(int x)
{
	int i;
	for (i=0;i<10;++i)
	{
		if (((x-i)>>3)>=dismap.size()||
			x-i<0)
			continue;
		if (dismap[(x-i)>>3]&
			(1<<((x-i)&7))
			)
			break;
	}
	if (i!=10)
	{
		if (DisasmLen(x-i)>i)
			x-=i;
	}
	return x;
}

void IdaGo(int dest)
{
	char address[15];
	sprintf(address,"%X",dest);
	FILE *fida=fopen("for ida.txt","w+");
	fprintf(fida,address);
	fclose(fida);
	HWND ida1=NULL,ida;
	for(;;)
	{
		ida1=FindWindowEx(NULL,ida1,"TIdaWindow",NULL);
		if (!ida1)
			break;
		ida=FindWindowEx(ida1,NULL,"MDIClient",NULL);
		ida=FindWindowEx(ida,NULL,"TMDIForm",NULL);
		ida=FindWindowEx(ida,NULL,"IdaView",NULL);
		PostMessage(ida,WM_KEYDOWN,'Z',NULL);
		PostMessage(ida,WM_KEYUP,'Z',NULL);
	}
}

void DebugWindow::Window()
{
	if (!HWnd)
		CreateDialog(NULL,MAKEINTRESOURCE(IDD_DEBUG),NULL,DebugProc);
	else
		SetForegroundWindow(HWnd);
}

void DebugWindow::ShowAddress(int dest)
{
	Window();
	if (!IsShowedAddress(dest))
		SetDisasmPos(dest);
	//else if (IdaSync)
	//	IdaGo(dest);
	Update();
}

void DebugWindow::SetDisasmPos(int x)
{
	if (IdaSync)
		IdaGo(x);
	SetScrollPos(GetDlgItem(HWnd,IDC_SCROLLBAR1),SB_CTL,x,FALSE);
}

void DebugWindow::UpdateBreak(int n)
{
	SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_DELETESTRING,(WPARAM)n,NULL);
	char buff[20];
	::Breakpoint &b=Breakpoints[n];
	sprintf(buff,"%c %06X-%06X %c%c%c",(b.enabled?'x':' '),b.start,b.end,(b.type&1?'p':' '),(b.type&2?'r':' '),(b.type&4?'w':' '));
	SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_INSERTSTRING,(WPARAM)n,(LPARAM)buff);
	SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_SETCURSEL,(WPARAM)n,NULL);
}

void DebugWindow::Breakpoint(int pc)
{
	Update_RAM_Watch();
	Clear_Sound_Buffer();
	Put_Info((char*)(whyBreakpoint.c_str()));
	ShowAddress(pc);
	if (!DummyHWnd)
	{
		DummyHWnd=(HWND)1;
		MSG msg={0};
		for(;Gens_Running&&DummyHWnd;)
		{
			/*if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (msg.hwnd==DebugWindowHWnd&&msg.message==WM_DEBUG_DUMMY_EXIT)
				{
					GetMessage(&msg, NULL, 0, 0);
					DispatchMessage(&msg); 
					break;
				}
				else
					Handle_Gens_Messages();
			}*/
			Handle_Gens_Messages();
		}
		//DebugDummyHWnd=(HWND)0;
	}
}

void DebugWindow::SetWhyBreak(LPCSTR lpString)
{
	whyBreakpoint = lpString;
	if (HWnd)
		SetDlgItemText(HWnd,IDC_WHY_BREAK,lpString);
}

LRESULT CALLBACK DebugWindow::Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;

	switch(uMsg)
	{
		case WM_INITDIALOG: {
			RECT r2;
			int dx1, dy1, dx2, dy2;

			HDC hdc = GetDC(hDlg);
 			MemDC=CreateCompatibleDC(hdc);
			MemBMP=CreateCompatibleBitmap(hdc,DEBUG_DISASM_WIDTH,DEBUG_DISASM_HEIGHT);
			LOGFONT lf;
			memset(&lf,0,sizeof(lf));
			lf.lfHeight=-MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			lf.lfWeight=700;
			lf.lfCharSet=DEFAULT_CHARSET;
			lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
			lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
			lf.lfQuality=DEFAULT_QUALITY;
			lf.lfPitchAndFamily=DEFAULT_PITCH;
			strcpy(lf.lfFaceName,"Courier New");
			Fonts[1] = CreateFontIndirect( &lf ); 
			LastBMP=(HBITMAP)SelectObject(MemDC,MemBMP);
			Fonts[0]=(HFONT)SelectObject(MemDC,Fonts[1]);

			HWnd = hDlg;

			for (dx1=0;dx1<Breakpoints.size();++dx1)
			{
				SendDlgItemMessage(hDlg,IDC_BREAK_LIST,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"");
				UpdateBreak(dx1);
			}

			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}

			GetWindowRect(::HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			// push it away from the main window if we can
			const int width = (r.right-r.left); 
			const int width2 = (r2.right-r2.left); 
			if(r.left+width2 + width < GetSystemMetrics(SM_CXSCREEN))
			{
				r.right += width;
				r.left += width;
			}
			else if((int)r.left - (int)width2 > 0)
			{
				r.right -= width2;
				r.left -= width2;
			}

			SetWindowText(hDlg,Title);
			SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			SendDlgItemMessage(hDlg,IDC_IDA_CHECK,BM_SETCHECK,(WPARAM)IdaSync,NULL);
			SetScrollRange(GetDlgItem(hDlg,IDC_SCROLLBAR1),SB_CTL,ScrollMin,ScrollMax,TRUE);
			SetDisasmPos(0);
			SetDlgItemText(hDlg,IDC_WHY_BREAK,whyBreakpoint.c_str());
			Update();
			return true;
		}	break;

		case WM_KEYDOWN: {
			switch(wParam)
			{
			case VK_F6:
				PostMessage(hDlg,WM_COMMAND,IDC_STEP_OVER,NULL);
				break;
			case VK_F7:
				PostMessage(hDlg,WM_COMMAND,IDC_STEP_INTO,NULL);
				break;
			case VK_F5:
				PostMessage(hDlg,WM_COMMAND,IDC_RUN,NULL);
				break;
			}
		}	break;

		case WM_COMMAND: {
			int msg=HIWORD(wParam);
			int id=LOWORD(wParam);
			switch(id)
			{
			case IDC_STEP_INTO:
				StepInto=true;
			case IDC_RUN:
				if (DummyHWnd)
					PostMessage(HWnd,WM_DEBUG_DUMMY_EXIT,NULL,NULL);
				break;

			case IDC_STEP_OVER:
				DoStepOver();
				PostMessage(HWnd,WM_DEBUG_DUMMY_EXIT,NULL,NULL);
				break;

			case IDC_ADD_BREAK: {
				int n=Breakpoints.size();
				Breakpoints.resize(n+1);
				::Breakpoint &b=Breakpoints[n];
				memset(&b,0,sizeof(::Breakpoint));
				SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"");
				UpdateBreak(n);
				SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_SETCURSEL,(WPARAM)n,NULL);
			}	//WARNING WITHOUT BREAK

			case IDC_EDIT_BREAK: {
				int n=SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_GETCURSEL,NULL,NULL);
				if (n!=LB_ERR)
				{
					DialogBoxParam(NULL,MAKEINTRESOURCE(IDD_EDIT_BREAK),HWnd,(DLGPROC)EditBreakProc,(LPARAM)&Breakpoints[n]);
					UpdateBreak(n);
				}
			}	break;

			case IDC_DEL_BREAK: {
				int n=SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_GETCURSEL,NULL,NULL);
				if (n!=LB_ERR)
				{
					SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_DELETESTRING,n,NULL);
					Breakpoints.erase(Breakpoints.begin()+n);
					if (n<Breakpoints.size())
						SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_SETCURSEL,(WPARAM)n,NULL);
					else if (Breakpoints.size())
						SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_SETCURSEL,(WPARAM)n-1,NULL);
				}
			}	break;

			case IDC_BREAK_LIST: {
				if (msg==LBN_DBLCLK)
				{
					int n=SendDlgItemMessage(HWnd,IDC_BREAK_LIST,LB_GETCURSEL,NULL,NULL);
					if (n!=LB_ERR)
					{
						Breakpoints[n].enabled=!Breakpoints[n].enabled;
						UpdateBreak(n);
					}
				}
			}	break;

			case IDC_CALL_STACK: {
				if (msg==LBN_DBLCLK)
				{
					int n=SendDlgItemMessage(HWnd,IDC_CALL_STACK,LB_GETCURSEL,NULL,NULL);
					if (n!=LB_ERR)
					{
						SetDisasmPos(callstack[callstack.size()-1-n]);
						Update();
					}
				}
			}	break;

			case IDC_LOAD_MAP: {
				char fname[2048];
				strcpy(fname,Rom_Name);
				strcat(fname,".map");
				if (Change_File_L(fname,".","Save Code Map As...","All Files\0*.*\0\0","*.*",hDlg))
				{
					LoadMap(fname);
				}
			}	break;

			case IDC_SAVE_MAP: {
				char fname[2048];
				strcpy(fname,Rom_Name);
				strcat(fname,".map");
				if (Change_File_S(fname,".","Save Code Map As...","All Files\0*.*\0\0","*.*",hDlg))
				{
					SaveMap(fname);
				}
			}	break;

			case IDC_IDA_CHECK: {
				IdaSync=!IdaSync;
				SendDlgItemMessage(hDlg,IDC_IDA_CHECK,BM_SETCHECK,(WPARAM)IdaSync,NULL);
			}	break;

			}

		}	break;

		case WM_LBUTTONDOWN:
		{
			short x=LOWORD(lParam);
			short y=HIWORD(lParam);
			SelectedLine=(y-5)/18;
			Update();
			SetForegroundWindow(hDlg);
			/*if(!SetFocus(hDlg))
			{
				DWORD xz=GetLastError();
				char buuf[20];
				sprintf(buuf,"%x",xz);
				MessageBox(NULL,buuf,"asd",MB_OK);
			}
			if (GetFocus()!=hDlg)
				hDlg=(HWND)1;*/
		}	break;

		case WM_VSCROLL:
		{
			int CurPos=GetScrollPos(GetDlgItem(hDlg,IDC_SCROLLBAR1),SB_CTL);
			int nSBCode=LOWORD(wParam);
			int nPos=HIWORD(wParam);
			switch (nSBCode)
			{
			case SB_LEFT:      // Scroll to far left.
				CurPos = 0;
				break;

			case SB_RIGHT:      // Scroll to far right.
				CurPos = 10000000;
				break;

			case SB_ENDSCROLL:   // End scroll.
				break;

			case SB_LINELEFT:      // Scroll left.
				CurPos=GetNearestScroll(GetNearestScroll(CurPos)-1);
				break;

			case SB_LINERIGHT:   // Scroll right.
				CurPos=GetNearestScroll(CurPos);
				if (CurPos>=0&&
					(CurPos>>3)<dismap.size()&&
					(dismap[CurPos>>3]&(1<<(CurPos&7)))
					)
					CurPos+=DisasmLen(CurPos);
				else
					++CurPos;
				break;

			case SB_PAGELEFT:    // Scroll one page left.
				if (CurPos - 15 > 0)
					CurPos -= 15;
				break;

			case SB_PAGERIGHT:      // Scroll one page righ
				CurPos += 15;
				break;

			case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
			case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
			{
				SCROLLINFO si;
				ZeroMemory(&si, sizeof(si));
				si.cbSize = sizeof(si);
				si.fMask = SIF_TRACKPOS;
 
				// Call GetScrollInfo to get current tracking 
				//    position in si.nTrackPos
 
				if (!GetScrollInfo(GetDlgItem(hDlg,IDC_SCROLLBAR1), SB_CTL, &si) )
					return 1; // GetScrollInfo failed
				CurPos = si.nTrackPos;
			}	break;
			}
			SetDisasmPos(CurPos);
			Update();
		}	break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hDlg,&ps);
			BitBlt(ps.hdc,5,5,DEBUG_DISASM_WIDTH,DEBUG_DISASM_HEIGHT,MemDC,0,0,SRCCOPY);
			EndPaint(hDlg,&ps);
			return true;
		}	break;

		case WM_CLOSE:
			if (Full_Screen)
			{
				while (ShowCursor(true) < 0);
				while (ShowCursor(false) >= 0);
			}
			SelectObject(MemDC, Fonts[0]);
			SelectObject(MemDC, LastBMP);
			DeleteObject(MemBMP);
			DeleteObject(Fonts[1]);
			DeleteObject(MemDC);
			EndDialog(hDlg, true);
			HWnd=NULL;
			return true;
		case WM_DEBUG_DUMMY_EXIT:
			DummyHWnd=NULL;
			return true;
	}

	return false;
}

void DebugWindow::ResetMap()
{
	memset(&dismap[0],0,dismap.size());
}

void DebugWindow::LoadMap(char *fname)
{
	FILE *in=fopen(fname,"rb");
	fread(&dismap[0],1,dismap.size(),in);
	fclose(in);
}

void DebugWindow::SaveMap(char *fname)
{
	remove(fname);
	FILE *out=fopen(fname,"wb");
	fclose(out);
	out=fopen(fname,"wb");
	fwrite(&dismap[0],1,dismap.size(),out);
	fclose(out);
}

LRESULT CALLBACK EditBreakProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	static Breakpoint *b=0;

	switch(uMsg)
	{
		case WM_INITDIALOG: {
			RECT r2;
			int dx1, dy1, dx2, dy2;

			b=(Breakpoint *)lParam;
			char buff[15];
			sprintf(buff,"%06X-%06X",b->start,b->end);
			SetDlgItemText(hDlg,IDC_EDIT_RANGE,buff);
			CheckDlgButton(hDlg,IDC_ENABLE,(b->enabled?BST_CHECKED:BST_UNCHECKED));
 			CheckDlgButton(hDlg,IDC_PC,(b->type&1?BST_CHECKED:BST_UNCHECKED));
			CheckDlgButton(hDlg,IDC_READ,(b->type&2?BST_CHECKED:BST_UNCHECKED));
			CheckDlgButton(hDlg,IDC_WRITE,(b->type&4?BST_CHECKED:BST_UNCHECKED));
			CheckDlgButton(hDlg,IDC_FORBID,(b->type&8?BST_CHECKED:BST_UNCHECKED));
			
			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			// push it away from the main window if we can
			const int width = (r.right-r.left); 
			const int width2 = (r2.right-r2.left); 
			if(r.left+width2 + width < GetSystemMetrics(SM_CXSCREEN))
			{
				r.right += width;
				r.left += width;
			}
			else if((int)r.left - (int)width2 > 0)
			{
				r.right -= width2;
				r.left -= width2;
			}

			SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

			return true;
		}	break;
		case WM_COMMAND: {
			switch(wParam)
			{
				case IDOK: {
					char buff[15];
					GetDlgItemText(hDlg,IDC_EDIT_RANGE,buff,sizeof(buff));
					if (sscanf(buff,"%x-%x",&(b->start),&(b->end))==1)
						b->end=b->start;
					if (b->end<b->start)
						b->end=b->start;
					b->enabled=(IsDlgButtonChecked(hDlg,IDC_ENABLE)==BST_CHECKED);
					b->type=(IsDlgButtonChecked(hDlg,IDC_PC)==BST_CHECKED)+
						((IsDlgButtonChecked(hDlg,IDC_READ)==BST_CHECKED)*2)+
						((IsDlgButtonChecked(hDlg,IDC_WRITE)==BST_CHECKED)*4)+
						((IsDlgButtonChecked(hDlg,IDC_FORBID)==BST_CHECKED)*8);
					EndDialog(hDlg,true);
				}	break;

				case IDCANCEL:
					EndDialog(hDlg,true);
					break;
			}
			return true;
		}	break;
		case WM_CLOSE:
			EndDialog(hDlg,true);
			return true;
			break;
	}
	return false;
}


bool DebugWindow::BreakPC(int pc)
{
	std::vector<::Breakpoint>::iterator i,n;
	i=Breakpoints.begin();
	n=Breakpoints.end();
	bool r=false;
	for (;i!=n;++i)
	{
		if (!(i->enabled)||!(i->type&1))
			continue;
		if (pc>=i->start &&
			pc<=i->end)
		{
			if (i->type&8)
				return false;
			r=true;
		}
	}
	return r;
}

bool DebugWindow::BreakRead(int pc,uint32 star,uint32 stop)
{
	std::vector<::Breakpoint>::iterator i,n;
	i=Breakpoints.begin();
	n=Breakpoints.end();
	bool r=false;
	for (;i!=n;++i)
	{
		if ((i->enabled)&&(i->type&8))
		{
			if ((i->type&1)&&
				pc>=i->start &&
				pc<=i->end)
				return false;
			if ((i->type&2)&&
				star>=i->start &&
				stop<=i->end)
				return false;
		}
		if (!(i->enabled)||!(i->type&2))
			continue;
		if (stop>=i->start &&
			star<=i->end)
			r=true;
	}
	return r;
}

bool DebugWindow::BreakWrite(int pc,uint32 star,uint32 stop)
{
	std::vector<::Breakpoint>::iterator i,n;
	i=Breakpoints.begin();
	n=Breakpoints.end();
	bool r=false;
	for (;i!=n;++i)
	{
		if ((i->enabled)&&(i->type&8))
		{
			if ((i->type&1)&&
				pc>=i->start &&
				pc<=i->end)
				return false;
			if ((i->type&4)&&
				star>=i->start &&
				stop<=i->end)
				return false;
		}
		if (!(i->enabled)||!(i->type&4))
			continue;
		if (stop>=i->start &&
			star<=i->end)
			r=true;
	}
	return r;
}
