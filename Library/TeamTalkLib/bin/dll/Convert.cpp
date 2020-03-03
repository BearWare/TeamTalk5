/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#if defined(WIN32)
#include <ace/config.h>
#endif

#include "Convert.h"
#include <ace/OS.h>
#include <ace/ACE.h>
#include <myace/MyACE.h>

#include <map>

#if defined(__APPLE__)

#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
#include <Carbon/Carbon.h>
#define MAC_CARBON 1
#endif

#endif /** __APPLE__ **/

#include <teamtalk/CodecCommon.h>

#if defined(ENABLE_OPUS)
#include <codec/OpusEncoder.h>
#endif

using namespace std;

//some inspiration found here: http://www.vmware.com/support/ws45/doc/devices_linux_kb_ws.html

enum TTKey
{
    TTKEY_UNKNOWN               = 0x00000000,
                                
    TTKEY_ESCAPE                = 0x00000001,

    TTKEY_1                     = 0x00000002,
    TTKEY_2                     = 0x00000003,
    TTKEY_3                     = 0x00000004,
    TTKEY_4                     = 0x00000005,
    TTKEY_5                     = 0x00000006,
    TTKEY_6                     = 0x00000007,
    TTKEY_7                     = 0x00000008,
    TTKEY_8                     = 0x00000009,
    TTKEY_9                     = 0x0000000a,
    TTKEY_0                     = 0x0000000b,

    TTKEY_MINUS                 = 0x0000000c,
    TTKEY_EQUAL                 = 0x0000000d,
    TTKEY_BACKSPACE             = 0x0000000e,
    TTKEY_TAB                   = 0x0000000f,

    TTKEY_Q                     = 0x00000010,
    TTKEY_W                     = 0x00000011,
    TTKEY_E                     = 0x00000012,
    TTKEY_R                     = 0x00000013,
    TTKEY_T                     = 0x00000014,
    TTKEY_Y                     = 0x00000015,
    TTKEY_U                     = 0x00000016,
    TTKEY_I                     = 0x00000017,
    TTKEY_O                     = 0x00000018,
    TTKEY_P                     = 0x00000019,
    TTKEY_BRACKETLEFT           = 0x0000001a,
    TTKEY_BRACKETRIGHT          = 0x0000001b,
    TTKEY_ENTER                 = 0x0000001c,   //big button
    TTKEY_LEFTCTRL              = 0x0000001d,

    TTKEY_A                     = 0x0000001e,
    TTKEY_S                     = 0x0000001f,
    TTKEY_D                     = 0x00000020,
    TTKEY_F                     = 0x00000021,
    TTKEY_G                     = 0x00000022,
    TTKEY_H                     = 0x00000023,
    TTKEY_J                     = 0x00000024,
    TTKEY_K                     = 0x00000025,
    TTKEY_L                     = 0x00000026,
    TTKEY_SEMICOLON             = 0x00000027,
    TTKEY_APOSTROPHE            = 0x00000028,
    TTKEY_GRAVE /* ` */          = 0x00000029,  

    TTKEY_LEFTSHIFT             = 0x0000002a,
    TTKEY_BACKSLASH             = 0x0000002b,
    TTKEY_Z                     = 0x0000002c,
    TTKEY_X                     = 0x0000002d,
    TTKEY_C                     = 0x0000002e,
    TTKEY_V                     = 0x0000002f,
    TTKEY_B                     = 0x00000030,
    TTKEY_N                     = 0x00000031,
    TTKEY_M                     = 0x00000032,
    TTKEY_COMMA                 = 0x00000033,
    TTKEY_PERIOD                = 0x00000034,
    TTKEY_SLASH                 = 0x00000035,
    TTKEY_RIGHTSHIFT            = 0x00000036,

    TTKEY_NUM_ASTERISK          = 0x00000037,

    TTKEY_LEFTALT               = 0x00000038,

    TTKEY_SPACE                 = 0x00000039,

    TTKEY_CAPSLOCK              = 0x0000003a,
    
    TTKEY_F1                    = 0x0000003b,
    TTKEY_F2                    = 0x0000003c,
    TTKEY_F3                    = 0x0000003d,
    TTKEY_F4                    = 0x0000003e,
    TTKEY_F5                    = 0x0000003f,
    TTKEY_F6                    = 0x00000040,
    TTKEY_F7                    = 0x00000041,
    TTKEY_F8                    = 0x00000042,
    TTKEY_F9                    = 0x00000043,
    TTKEY_F10                   = 0x00000044,

    TTKEY_NUM_NUMLOCK           = 0x00000045,

    TTKEY_SCROLLLOCK            = 0x00000046,
    TTKEY_NUM_7                 = 0x00000047,
    TTKEY_NUM_8                 = 0x00000048,
    TTKEY_NUM_9                 = 0x00000049,
    TTKEY_NUM_MINUS             = 0x0000004a,
    TTKEY_NUM_4                 = 0x0000004b,
    TTKEY_NUM_5                 = 0x0000004c,
    TTKEY_NUM_6                 = 0x0000004d,
    TTKEY_NUM_PLUS              = 0x0000004e,
    TTKEY_NUM_1                 = 0x0000004f,
    TTKEY_NUM_2                 = 0x00000050,
    TTKEY_NUM_3                 = 0x00000051,
    TTKEY_NUM_0                 = 0x00000052,
    TTKEY_NUM_PERIOD            = 0x00000053,

    TTKEY_LT                    = 0x00000056, //on non-US keyboards (right of left shift)

    TTKEY_F11                   = 0x00000057,
    TTKEY_F12                   = 0x00000058,

    TTKEY_BREAK                 = 0x00000100,
    TTKEY_NUM_RETURN            = 0x0000011c,    //num key
    TTKEY_RIGHTCTRL             = 0x0000011d,
    TTKEY_NUM_SLASH             = 0x00000135,

    TTKEY_SYSRQ                 = 0x00000137,
    TTKEY_RIGHTALT              = 0x00000138,
    TTKEY_HOME                  = 0x00000147,
    TTKEY_UP                    = 0x00000148,
    TTKEY_PAGEUP                = 0x00000149,
    TTKEY_LEFT                  = 0x0000014b,
    TTKEY_RIGHT                 = 0x0000014d,
    TTKEY_END                   = 0x0000014f,
    TTKEY_DOWN                  = 0x00000150,
    TTKEY_PAGEDOWN              = 0x00000151,
    TTKEY_INSERT                = 0x00000152,
    TTKEY_DELETE                = 0x00000153,
    TTKEY_LEFTWIN               = 0x0000015b,
    TTKEY_RIGHTWIN              = 0x0000015c,
    TTKEY_MENU                  = 0x0000015d,


    /* mouse buttons not in VMware reference */
    TTKEY_LEFTMOUSEBTN          = 0x00001000,
    TTKEY_RIGHTMOUSEBTN         = 0x00001001,
    TTKEY_MIDDLEMOUSEBTN        = 0x00001002,
};

typedef std::map<ACE_UINT32, ACE_UINT32> key_map_t;

key_map_t tt_keymap, local_keymap;

#if defined(WIN32)

/* 0-15	The repeat count for the current message. The value
* is the number of times the keystroke is autorepeated as
* a result of the user holding down the key. If the
* keystroke is held long enough, multiple messages are sent.
* However, the repeat count is not cumulative.
*
* 16 - 23	The scan code.The value depends on the OEM.
*
* 24	Indicates whether the key is an extended key, such
* as the right - hand ALT and CTRL keys that appear on an
* enhanced 101 - or 102 - key keyboard. The value is 1 if it is an extended key; otherwise, it is 0.
*/

void FillKeys()
{
    tt_keymap[TTKEY_ESCAPE] = TTKEY_ESCAPE;

    tt_keymap[TTKEY_1] = TTKEY_1;
    tt_keymap[TTKEY_2] = TTKEY_2;
    tt_keymap[TTKEY_3] = TTKEY_3;
    tt_keymap[TTKEY_4] = TTKEY_4;
    tt_keymap[TTKEY_5] = TTKEY_5;
    tt_keymap[TTKEY_6] = TTKEY_6;
    tt_keymap[TTKEY_7] = TTKEY_7;
    tt_keymap[TTKEY_8] = TTKEY_8;
    tt_keymap[TTKEY_9] = TTKEY_9;
    tt_keymap[TTKEY_0] = TTKEY_0;

    tt_keymap[TTKEY_MINUS] = TTKEY_MINUS; //right of 0
    tt_keymap[TTKEY_EQUAL] = TTKEY_EQUAL; //left of backspace
    tt_keymap[TTKEY_BACKSPACE] = TTKEY_BACKSPACE;
    tt_keymap[TTKEY_TAB] = TTKEY_TAB;

    tt_keymap[TTKEY_Q] = TTKEY_Q;
    tt_keymap[TTKEY_W] = TTKEY_W;
    tt_keymap[TTKEY_E] = TTKEY_E;
    tt_keymap[TTKEY_R] = TTKEY_R;
    tt_keymap[TTKEY_T] = TTKEY_T;
    tt_keymap[TTKEY_Y] = TTKEY_Y;
    tt_keymap[TTKEY_U] = TTKEY_U;
    tt_keymap[TTKEY_I] = TTKEY_I;
    tt_keymap[TTKEY_O] = TTKEY_O;
    tt_keymap[TTKEY_P] = TTKEY_P;
    tt_keymap[TTKEY_BRACKETLEFT] = TTKEY_BRACKETLEFT; //right of P
    tt_keymap[TTKEY_BRACKETRIGHT] = TTKEY_BRACKETRIGHT; //right again
    tt_keymap[TTKEY_ENTER] = TTKEY_ENTER;
    tt_keymap[TTKEY_LEFTCTRL] = TTKEY_LEFTCTRL;
    
    tt_keymap[TTKEY_A] = TTKEY_A;
    tt_keymap[TTKEY_S] = TTKEY_S;
    tt_keymap[TTKEY_D] = TTKEY_D;
    tt_keymap[TTKEY_F] = TTKEY_F;
    tt_keymap[TTKEY_G] = TTKEY_G;
    tt_keymap[TTKEY_H] = TTKEY_H;
    tt_keymap[TTKEY_J] = TTKEY_J;
    tt_keymap[TTKEY_K] = TTKEY_K;
    tt_keymap[TTKEY_L] = TTKEY_L;
    tt_keymap[TTKEY_SEMICOLON] = TTKEY_SEMICOLON; //right of L
    tt_keymap[TTKEY_APOSTROPHE] = TTKEY_APOSTROPHE; //right again
    tt_keymap[TTKEY_GRAVE] = TTKEY_GRAVE; //left of 1
    
    tt_keymap[TTKEY_LEFTSHIFT] = TTKEY_LEFTSHIFT;
    tt_keymap[TTKEY_BACKSLASH] = TTKEY_BACKSLASH; //left of Z
    tt_keymap[TTKEY_Z] = TTKEY_Z;
    tt_keymap[TTKEY_X] = TTKEY_X;
    tt_keymap[TTKEY_C] = TTKEY_C;
    tt_keymap[TTKEY_V] = TTKEY_V;
    tt_keymap[TTKEY_B] = TTKEY_B;
    tt_keymap[TTKEY_N] = TTKEY_N;
    tt_keymap[TTKEY_M] = TTKEY_M;
    tt_keymap[TTKEY_COMMA] = TTKEY_COMMA; //right of M
    tt_keymap[TTKEY_PERIOD] = TTKEY_PERIOD; //right again
    tt_keymap[TTKEY_SLASH] = TTKEY_SLASH; //right again
    tt_keymap[TTKEY_RIGHTSHIFT] = TTKEY_RIGHTSHIFT;

    tt_keymap[TTKEY_NUM_ASTERISK] = TTKEY_NUM_ASTERISK;

    tt_keymap[TTKEY_LEFTALT] = TTKEY_LEFTALT;

    tt_keymap[TTKEY_SPACE] = TTKEY_SPACE;

    tt_keymap[TTKEY_CAPSLOCK] = TTKEY_CAPSLOCK;

    tt_keymap[TTKEY_F1] = TTKEY_F1;
    tt_keymap[TTKEY_F2] = TTKEY_F2;
    tt_keymap[TTKEY_F3] = TTKEY_F3;
    tt_keymap[TTKEY_F4] = TTKEY_F4;
    tt_keymap[TTKEY_F5] = TTKEY_F5;
    tt_keymap[TTKEY_F6] = TTKEY_F6;
    tt_keymap[TTKEY_F7] = TTKEY_F7;
    tt_keymap[TTKEY_F8] = TTKEY_F8;
    tt_keymap[TTKEY_F9] = TTKEY_F9;
    tt_keymap[TTKEY_F10] = TTKEY_F10;

    tt_keymap[TTKEY_NUM_NUMLOCK] = TTKEY_NUM_NUMLOCK;

    tt_keymap[TTKEY_SCROLLLOCK] = TTKEY_SCROLLLOCK;
    tt_keymap[TTKEY_NUM_7] = TTKEY_NUM_7;
    tt_keymap[TTKEY_NUM_8] = TTKEY_NUM_8;
    tt_keymap[TTKEY_NUM_9] = TTKEY_NUM_9;
    tt_keymap[TTKEY_NUM_MINUS] = TTKEY_NUM_MINUS;
    tt_keymap[TTKEY_NUM_4] = TTKEY_NUM_4;
    tt_keymap[TTKEY_NUM_5] = TTKEY_NUM_5;
    tt_keymap[TTKEY_NUM_6] = TTKEY_NUM_6;
    tt_keymap[TTKEY_NUM_PLUS] = TTKEY_NUM_PLUS;
    tt_keymap[TTKEY_NUM_1] = TTKEY_NUM_1;
    tt_keymap[TTKEY_NUM_2] = TTKEY_NUM_2;
    tt_keymap[TTKEY_NUM_3] = TTKEY_NUM_3;
    tt_keymap[TTKEY_NUM_0] = TTKEY_NUM_0;
    tt_keymap[TTKEY_NUM_PERIOD] = TTKEY_NUM_PERIOD;

    tt_keymap[TTKEY_LT] = TTKEY_LT;

    tt_keymap[TTKEY_F11] = TTKEY_F11;
    tt_keymap[TTKEY_F12] = TTKEY_F12;

    tt_keymap[TTKEY_BREAK] = TTKEY_BREAK;
    tt_keymap[TTKEY_NUM_RETURN] = TTKEY_NUM_RETURN;
    tt_keymap[TTKEY_RIGHTCTRL] = TTKEY_RIGHTCTRL;
    tt_keymap[TTKEY_NUM_SLASH] = TTKEY_NUM_SLASH;

    tt_keymap[TTKEY_SYSRQ] = TTKEY_SYSRQ;
    tt_keymap[TTKEY_RIGHTALT] = TTKEY_RIGHTALT;
    tt_keymap[TTKEY_HOME] = TTKEY_HOME;
    tt_keymap[TTKEY_UP] = TTKEY_UP;
    tt_keymap[TTKEY_PAGEUP] = TTKEY_PAGEUP;
    tt_keymap[TTKEY_LEFT] = TTKEY_LEFT;
    tt_keymap[TTKEY_RIGHT] = TTKEY_RIGHT;
    tt_keymap[TTKEY_END] = TTKEY_END;
    tt_keymap[TTKEY_DOWN] = TTKEY_DOWN;
    tt_keymap[TTKEY_PAGEDOWN] = TTKEY_PAGEDOWN;
    tt_keymap[TTKEY_INSERT] = TTKEY_INSERT;
    tt_keymap[TTKEY_DELETE] = TTKEY_DELETE;
    tt_keymap[TTKEY_LEFTWIN] = TTKEY_LEFTWIN;
    tt_keymap[TTKEY_RIGHTWIN] = TTKEY_RIGHTWIN;
    tt_keymap[TTKEY_MENU] = TTKEY_MENU;

    tt_keymap[TTKEY_LEFTMOUSEBTN] = TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN;
    tt_keymap[TTKEY_RIGHTMOUSEBTN] = TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN;
    tt_keymap[TTKEY_MIDDLEMOUSEBTN] = TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN;
/*
    tt_keymap[TTKEY_ESCAPE] = VK_ESCAPE;

    tt_keymap[TTKEY_1] = '1';
    tt_keymap[TTKEY_2] = '2';
    tt_keymap[TTKEY_3] = '3';
    tt_keymap[TTKEY_4] = '4';
    tt_keymap[TTKEY_5] = '5';
    tt_keymap[TTKEY_6] = '6';
    tt_keymap[TTKEY_7] = '7';
    tt_keymap[TTKEY_8] = '8';
    tt_keymap[TTKEY_9] = '9';
    tt_keymap[TTKEY_0] = '0';

    //tt_keymap[TTKEY_MINUS] = ; //right of 0
    //tt_keymap[TTKEY_EQUAL] = ; //left of backspace
    tt_keymap[TTKEY_BACKSPACE] = VK_BACK;
    tt_keymap[TTKEY_TAB] = VK_TAB;

    tt_keymap[TTKEY_Q] = 'Q';
    tt_keymap[TTKEY_W] = 'W';
    tt_keymap[TTKEY_E] = 'E';
    tt_keymap[TTKEY_R] = 'R';
    tt_keymap[TTKEY_T] = 'T';
    tt_keymap[TTKEY_Y] = 'Y';
    tt_keymap[TTKEY_U] = 'U';
    tt_keymap[TTKEY_I] = 'I';
    tt_keymap[TTKEY_O] = 'O';
    tt_keymap[TTKEY_P] = 'P';
    //tt_keymap[TTKEY_BRACKETLEFT] = ; //right of P
    //tt_keymap[TTKEY_BRACKETRIGHT] = ; //right again
    tt_keymap[TTKEY_ENTER] = VK_RETURN;
    tt_keymap[TTKEY_LEFTCTRL] = VK_LCONTROL;
    
    tt_keymap[TTKEY_A] = 'A';
    tt_keymap[TTKEY_S] = 'S';
    tt_keymap[TTKEY_D] = 'D';
    tt_keymap[TTKEY_F] = 'F';
    tt_keymap[TTKEY_G] = 'G';
    tt_keymap[TTKEY_H] = 'H';
    tt_keymap[TTKEY_J] = 'J';
    tt_keymap[TTKEY_K] = 'K';
    tt_keymap[TTKEY_L] = 'L';
    //tt_keymap[TTKEY_SEMICOLON] = ; //right of L
    //tt_keymap[TTKEY_APOSTROPHE] = ; //right again
    //tt_keymap[TTKEY_GRAVE] = ; //left of 1
    
    tt_keymap[TTKEY_LEFTSHIFT] = VK_LSHIFT;
    //tt_keymap[TTKEY_BACKSLASH] = ; //left of Z
    tt_keymap[TTKEY_Z] = 'Z';
    tt_keymap[TTKEY_X] = 'X';
    tt_keymap[TTKEY_C] = 'C';
    tt_keymap[TTKEY_V] = 'V';
    tt_keymap[TTKEY_B] = 'B';
    tt_keymap[TTKEY_N] = 'N';
    tt_keymap[TTKEY_M] = 'M';
    //tt_keymap[TTKEY_COMMA] = ; //right of M
    //tt_keymap[TTKEY_PERIOD] = ; //right again
    //tt_keymap[TTKEY_SLASH] = ; //right again
    tt_keymap[TTKEY_RIGHTSHIFT] = VK_RSHIFT;

    tt_keymap[TTKEY_NUM_ASTERISK] = VK_MULTIPLY;

    tt_keymap[TTKEY_LEFTALT] = VK_LMENU;

    tt_keymap[TTKEY_SPACE] = VK_SPACE;

    tt_keymap[TTKEY_CAPSLOCK] = VK_CAPITAL;

    tt_keymap[TTKEY_F1] = VK_F1;
    tt_keymap[TTKEY_F2] = VK_F2;
    tt_keymap[TTKEY_F3] = VK_F3;
    tt_keymap[TTKEY_F4] = VK_F4;
    tt_keymap[TTKEY_F5] = VK_F5;
    tt_keymap[TTKEY_F6] = VK_F6;
    tt_keymap[TTKEY_F7] = VK_F7;
    tt_keymap[TTKEY_F8] = VK_F8;
    tt_keymap[TTKEY_F9] = VK_F9;
    tt_keymap[TTKEY_F10] = VK_F10;

    tt_keymap[TTKEY_NUM_NUMLOCK] = VK_NUMLOCK;

    tt_keymap[TTKEY_SCROLLLOCK] = VK_SCROLL;
    tt_keymap[TTKEY_NUM_7] = VK_NUMPAD7;
    tt_keymap[TTKEY_NUM_8] = VK_NUMPAD8;
    tt_keymap[TTKEY_NUM_9] = VK_NUMPAD9;
    tt_keymap[TTKEY_NUM_MINUS] = VK_SUBTRACT;
    tt_keymap[TTKEY_NUM_4] = VK_NUMPAD4;
    tt_keymap[TTKEY_NUM_5] = VK_NUMPAD5;
    tt_keymap[TTKEY_NUM_6] = VK_NUMPAD6;
    tt_keymap[TTKEY_NUM_PLUS] = VK_ADD;
    tt_keymap[TTKEY_NUM_1] = VK_NUMPAD1;
    tt_keymap[TTKEY_NUM_2] = VK_NUMPAD2;
    tt_keymap[TTKEY_NUM_3] = VK_NUMPAD3;
    tt_keymap[TTKEY_NUM_0] = VK_NUMPAD0;
    tt_keymap[TTKEY_NUM_PERIOD] = VK_DECIMAL;

    tt_keymap[TTKEY_F11] = VK_F11;
    tt_keymap[TTKEY_F12] = VK_F12;

    tt_keymap[TTKEY_BREAK] = VK_PAUSE;
    tt_keymap[TTKEY_NUM_RETURN] = VK_RETURN;
    tt_keymap[TTKEY_RIGHTCTRL] = VK_RCONTROL;
    tt_keymap[TTKEY_NUM_SLASH] = VK_DIVIDE;

    tt_keymap[TTKEY_SYSRQ] = VK_PRINT;
    tt_keymap[TTKEY_RIGHTALT] = VK_RMENU;
    tt_keymap[TTKEY_HOME] = VK_HOME;
    tt_keymap[TTKEY_UP] = VK_UP;
    tt_keymap[TTKEY_PAGEUP] = VK_PRIOR;
    tt_keymap[TTKEY_LEFT] = VK_LEFT;
    tt_keymap[TTKEY_RIGHT] = VK_RIGHT;
    tt_keymap[TTKEY_END] = VK_END;
    tt_keymap[TTKEY_DOWN] = VK_DOWN;
    tt_keymap[TTKEY_PAGEDOWN] = VK_NEXT;
    tt_keymap[TTKEY_INSERT] = VK_INSERT;
    tt_keymap[TTKEY_DELETE] = VK_DELETE;
    tt_keymap[TTKEY_LEFTWIN] = VK_LWIN;
    tt_keymap[TTKEY_RIGHTWIN] = VK_RWIN;
    tt_keymap[TTKEY_MENU] = VK_APPS;

    tt_keymap[TTKEY_LEFTMOUSEBTN] = VK_LBUTTON;
    tt_keymap[TTKEY_RIGHTMOUSEBTN] = VK_RBUTTON;
    tt_keymap[TTKEY_MIDDLEMOUSEBTN] = VK_MBUTTON;
*/
}
#elif defined(MAC_CARBON)
void FillKeys()
{
    tt_keymap[TTKEY_ESCAPE] = kVK_Escape;

    tt_keymap[TTKEY_1] = kVK_ANSI_1;
    tt_keymap[TTKEY_2] = kVK_ANSI_2;
    tt_keymap[TTKEY_3] = kVK_ANSI_3;
    tt_keymap[TTKEY_4] = kVK_ANSI_4;
    tt_keymap[TTKEY_5] = kVK_ANSI_5;
    tt_keymap[TTKEY_6] = kVK_ANSI_6;
    tt_keymap[TTKEY_7] = kVK_ANSI_7;
    tt_keymap[TTKEY_8] = kVK_ANSI_8;
    tt_keymap[TTKEY_9] = kVK_ANSI_9;
    tt_keymap[TTKEY_0] = kVK_ANSI_0;

    tt_keymap[TTKEY_MINUS] = kVK_ANSI_Minus; //right of 0
    tt_keymap[TTKEY_EQUAL] = kVK_ANSI_Equal; //left of backspace
    tt_keymap[TTKEY_BACKSPACE] = kVK_Delete;
    tt_keymap[TTKEY_TAB] = kVK_Tab;

    tt_keymap[TTKEY_Q] = kVK_ANSI_Q;
    tt_keymap[TTKEY_W] = kVK_ANSI_W;
    tt_keymap[TTKEY_E] = kVK_ANSI_E;
    tt_keymap[TTKEY_R] = kVK_ANSI_R;
    tt_keymap[TTKEY_T] = kVK_ANSI_T;
    tt_keymap[TTKEY_Y] = kVK_ANSI_Y;
    tt_keymap[TTKEY_U] = kVK_ANSI_U;
    tt_keymap[TTKEY_I] = kVK_ANSI_I;
    tt_keymap[TTKEY_O] = kVK_ANSI_O;
    tt_keymap[TTKEY_P] = kVK_ANSI_P;
    tt_keymap[TTKEY_BRACKETLEFT] = kVK_ANSI_LeftBracket; //right of P
    tt_keymap[TTKEY_BRACKETRIGHT] = kVK_ANSI_RightBracket; //right again
    tt_keymap[TTKEY_ENTER] = kVK_Return;
    tt_keymap[TTKEY_LEFTCTRL] = kVK_Control;
    
    tt_keymap[TTKEY_A] = kVK_ANSI_A;
    tt_keymap[TTKEY_S] = kVK_ANSI_S;
    tt_keymap[TTKEY_D] = kVK_ANSI_D;
    tt_keymap[TTKEY_F] = kVK_ANSI_F;
    tt_keymap[TTKEY_G] = kVK_ANSI_G;
    tt_keymap[TTKEY_H] = kVK_ANSI_H;
    tt_keymap[TTKEY_J] = kVK_ANSI_J;
    tt_keymap[TTKEY_K] = kVK_ANSI_K;
    tt_keymap[TTKEY_L] = kVK_ANSI_L;
    tt_keymap[TTKEY_SEMICOLON] = kVK_ANSI_Semicolon; //right of L
    tt_keymap[TTKEY_APOSTROPHE] = kVK_ANSI_Quote; //right again
    tt_keymap[TTKEY_GRAVE] = kVK_ANSI_Grave; //left of 1
    
    tt_keymap[TTKEY_LEFTSHIFT] = kVK_Shift;
    tt_keymap[TTKEY_BACKSLASH] = kVK_ANSI_Backslash; //left of Z
    tt_keymap[TTKEY_Z] = kVK_ANSI_Z;
    tt_keymap[TTKEY_X] = kVK_ANSI_X;
    tt_keymap[TTKEY_C] = kVK_ANSI_C;
    tt_keymap[TTKEY_V] = kVK_ANSI_V;
    tt_keymap[TTKEY_B] = kVK_ANSI_B;
    tt_keymap[TTKEY_N] = kVK_ANSI_N;
    tt_keymap[TTKEY_M] = kVK_ANSI_M;
    tt_keymap[TTKEY_COMMA] = kVK_ANSI_Comma; //right of M
    tt_keymap[TTKEY_PERIOD] = kVK_ANSI_Period; //right again
    tt_keymap[TTKEY_SLASH] = kVK_ANSI_Slash; //right again
    tt_keymap[TTKEY_RIGHTSHIFT] = kVK_RightShift;

    tt_keymap[TTKEY_NUM_ASTERISK] = kVK_ANSI_KeypadMultiply;

    tt_keymap[TTKEY_LEFTALT] = kVK_Option;

    tt_keymap[TTKEY_SPACE] = kVK_Space;

    tt_keymap[TTKEY_CAPSLOCK] = kVK_CapsLock;

    tt_keymap[TTKEY_F1] = kVK_F1;
    tt_keymap[TTKEY_F2] = kVK_F2;
    tt_keymap[TTKEY_F3] = kVK_F3;
    tt_keymap[TTKEY_F4] = kVK_F4;
    tt_keymap[TTKEY_F5] = kVK_F5;
    tt_keymap[TTKEY_F6] = kVK_F6;
    tt_keymap[TTKEY_F7] = kVK_F7;
    tt_keymap[TTKEY_F8] = kVK_F8;
    tt_keymap[TTKEY_F9] = kVK_F9;
    tt_keymap[TTKEY_F10] = kVK_F10;

//     tt_keymap[TTKEY_NUM_NUMLOCK] = VK_NUMLOCK;

//     tt_keymap[TTKEY_SCROLLLOCK] = VK_SCROLL;
    tt_keymap[TTKEY_NUM_7] = kVK_ANSI_Keypad7;
    tt_keymap[TTKEY_NUM_8] = kVK_ANSI_Keypad8;
    tt_keymap[TTKEY_NUM_9] = kVK_ANSI_Keypad9;
    tt_keymap[TTKEY_NUM_MINUS] = kVK_ANSI_KeypadMinus;
    tt_keymap[TTKEY_NUM_4] = kVK_ANSI_Keypad4;
    tt_keymap[TTKEY_NUM_5] = kVK_ANSI_Keypad5;
    tt_keymap[TTKEY_NUM_6] = kVK_ANSI_Keypad6;
    tt_keymap[TTKEY_NUM_PLUS] = kVK_ANSI_KeypadPlus;
    tt_keymap[TTKEY_NUM_1] = kVK_ANSI_Keypad1;
    tt_keymap[TTKEY_NUM_2] = kVK_ANSI_Keypad2;
    tt_keymap[TTKEY_NUM_3] = kVK_ANSI_Keypad3;
    tt_keymap[TTKEY_NUM_0] = kVK_ANSI_Keypad0;
    tt_keymap[TTKEY_NUM_PERIOD] = kVK_ANSI_KeypadDecimal;

    tt_keymap[TTKEY_F11] = kVK_F11;
    tt_keymap[TTKEY_F12] = kVK_F12;

//     tt_keymap[TTKEY_BREAK] = VK_PAUSE;
     tt_keymap[TTKEY_NUM_RETURN] = kVK_Return;
     tt_keymap[TTKEY_RIGHTCTRL] = kVK_RightControl;
     tt_keymap[TTKEY_NUM_SLASH] = kVK_ANSI_KeypadDivide;

//     tt_keymap[TTKEY_SYSRQ] = VK_PRINT;
     tt_keymap[TTKEY_RIGHTALT] = kVK_RightOption;
     tt_keymap[TTKEY_HOME] = kVK_Home;
     tt_keymap[TTKEY_UP] = kVK_UpArrow;
     tt_keymap[TTKEY_PAGEUP] = kVK_PageUp;
     tt_keymap[TTKEY_LEFT] = kVK_LeftArrow;
     tt_keymap[TTKEY_RIGHT] = kVK_RightArrow;
     tt_keymap[TTKEY_END] = kVK_End;
     tt_keymap[TTKEY_DOWN] = kVK_DownArrow;
     tt_keymap[TTKEY_PAGEDOWN] = kVK_PageDown;
//     tt_keymap[TTKEY_INSERT] = VK_INSERT;
     tt_keymap[TTKEY_DELETE] = kVK_ForwardDelete;
     tt_keymap[TTKEY_LEFTWIN] = kVK_Command;
     tt_keymap[TTKEY_RIGHTWIN] = kVK_Command;
//     tt_keymap[TTKEY_MENU] = VK_APPS;

    tt_keymap[TTKEY_LEFTMOUSEBTN] = TT_DESKTOPINPUT_KEYCODE_LMOUSEBTN;
    tt_keymap[TTKEY_RIGHTMOUSEBTN] = TT_DESKTOPINPUT_KEYCODE_RMOUSEBTN;
    tt_keymap[TTKEY_MIDDLEMOUSEBTN] = TT_DESKTOPINPUT_KEYCODE_MMOUSEBTN;
}

#elif defined(__ANDROID_API__)

//TODO: key translate for Android
void FillKeys() {}

#else

//TODO: X11 key translate

void FillKeys() {}

/*
#include <X11/keysym.h>

void FillKeys()
{
    //usr/include/X11/keysymdef.h

    tt_keymap[TTKEY_ESCAPE] = XK_Escape;
    tt_keymap[TTKEY_TAB] = XK_Tab;
    tt_keymap[TTKEY_BACKSPACE] = XK_BackSpace;
    tt_keymap[TTKEY_RETURN] = XK_Return;
    tt_keymap[TTKEY_ENTER] = 
    tt_keymap[TTKEY_INSERT] = XK_Insert;
    tt_keymap[TTKEY_DELETE] = 
        tt_keymap[TTKEY_PAUSE] = XK_Pause;
    tt_keymap[TTKEY_PRINT] = 

        tt_keymap[TTKEY_HOME] = XK_Home;
    tt_keymap[TTKEY_END] = XK_End
    tt_keymap[TTKEY_LEFT] = XK_Left
    tt_keymap[TTKEY_UP] = XK_Up
    tt_keymap[TTKEY_RIGHT] = XK_Right
    tt_keymap[TTKEY_DOWN] = XK_Down
    tt_keymap[TTKEY_PAGEUP] = XK_Page_Up
    tt_keymap[TTKEY_PAGEDOWN] = XK_Page_Down
    tt_keymap[TTKEY_SHIFT] = 
    tt_keymap[TTKEY_CONTROL] = 
    tt_keymap[TTKEY_META] = 
    tt_keymap[TTKEY_ALT] = 
    tt_keymap[TTKEY_ALTGR] = 

    tt_keymap[TTKEY_CAPSLOCK] = 
    tt_keymap[TTKEY_NUMLOCK] = XK_Num_Lock
    tt_keymap[TTKEY_SCROLLLOCK] = XK_Scroll_Lock
    tt_keymap[TTKEY_F1] = XK_F1
    tt_keymap[TTKEY_F2] = XK_F2
    tt_keymap[TTKEY_F3] = XK_F3
    tt_keymap[TTKEY_F4] = XK_F4
    tt_keymap[TTKEY_F5] = XK_F5
    tt_keymap[TTKEY_F6] = XK_F6
    tt_keymap[TTKEY_F7] = XK_F7
    tt_keymap[TTKEY_F8] = XK_F8
    tt_keymap[TTKEY_F9] = XK_F9
    tt_keymap[TTKEY_F10] = XK_F10
    tt_keymap[TTKEY_F11] = XK_F11
    tt_keymap[TTKEY_F12] = XK_F12

    tt_keymap[TTKEY_SPACE] = 

    tt_keymap[TTKEY_APOSTROPHE] = 

    tt_keymap[TTKEY_ASTERISK] = 
    tt_keymap[TTKEY_PLUS] = 
    tt_keymap[TTKEY_COMMA] = 
    tt_keymap[TTKEY_MINUS] = 
    tt_keymap[TTKEY_PERIOD] = 
    tt_keymap[TTKEY_SLASH] = 
    tt_keymap[TTKEY_0] = 
    tt_keymap[TTKEY_1] = 
    tt_keymap[TTKEY_2] = 
    tt_keymap[TTKEY_3] = 
    tt_keymap[TTKEY_4] = 
    tt_keymap[TTKEY_5] = 
    tt_keymap[TTKEY_6] = 
    tt_keymap[TTKEY_7] = 
    tt_keymap[TTKEY_8] = 
    tt_keymap[TTKEY_9] = 
    tt_keymap[TTKEY_COLON] = 
    tt_keymap[TTKEY_SEMICOLON] = 
    tt_keymap[TTKEY_LESS]] = 
    tt_keymap[TTKEY_EQUAL] = 

    tt_keymap[TTKEY_A] = 
    tt_keymap[TTKEY_B] = 
    tt_keymap[TTKEY_C] = 
    tt_keymap[TTKEY_D] = 
    tt_keymap[TTKEY_E] = 
    tt_keymap[TTKEY_F] = 
    tt_keymap[TTKEY_G] = 
    tt_keymap[TTKEY_H] = 
    tt_keymap[TTKEY_I] = 
    tt_keymap[TTKEY_J] = 
    tt_keymap[TTKEY_K] = 
    tt_keymap[TTKEY_L] = 
    tt_keymap[TTKEY_M] = 
    tt_keymap[TTKEY_N] = 
    tt_keymap[TTKEY_O] = 
    tt_keymap[TTKEY_P] = 
    tt_keymap[TTKEY_Q] = 
    tt_keymap[TTKEY_R] = 
    tt_keymap[TTKEY_S] = 
    tt_keymap[TTKEY_T] = 
    tt_keymap[TTKEY_U] = 
    tt_keymap[TTKEY_V] = 
    tt_keymap[TTKEY_W] = 
    tt_keymap[TTKEY_X] = 
    tt_keymap[TTKEY_Y] = 
    tt_keymap[TTKEY_Z] = 
    tt_keymap[TTKEY_BRACKETLEFT] = 
    tt_keymap[TTKEY_BACKSLASH] = 
    tt_keymap[TTKEY_BRACKETRIGHT] = 

}
*/
#endif

ACE_Recursive_Thread_Mutex keytransMtx;

bool TranslateDesktopInput(TTKeyTranslate nTranslate,
                           const DesktopInput& input,
                           DesktopInput& output)
{
    key_map_t::const_iterator ii;
    if(tt_keymap.empty())
    {
        wguard_t g(keytransMtx);
        FillKeys();
        ii = tt_keymap.begin();
        for(;ii!=tt_keymap.end();ii++)
            local_keymap[ii->second] = ii->first;
    }

    output = input;

    if(input.uKeyCode == TT_DESKTOPINPUT_KEYCODE_IGNORE)
        return true;

    switch(nTranslate)
    {
#if defined(WIN32)
    case TTKEY_WINKEYCODE_TO_TTKEYCODE :
#endif
#if defined(MAC_CARBON)
    case TTKEY_MACKEYCODE_TO_TTKEYCODE :
#endif
        output.uKeyCode = ( ((ii = local_keymap.find(input.uKeyCode)) != local_keymap.end())?
                           ii->second : TT_DESKTOPINPUT_KEYCODE_IGNORE);
    break;
#if defined(WIN32)
    case TTKEY_TTKEYCODE_TO_WINKEYCODE :
#endif
#if defined(MAC_CARBON)
    case TTKEY_TTKEYCODE_TO_MACKEYCODE :
#endif
        output.uKeyCode = (ii = tt_keymap.find(input.uKeyCode)) != tt_keymap.end()?
            ii->second : TT_DESKTOPINPUT_KEYCODE_IGNORE;
        break;
    case TTKEY_NO_TRANSLATE :
        break;
    default :
        output.uKeyCode = TT_DESKTOPINPUT_KEYCODE_IGNORE;
        break;
    }
    return output.uKeyCode != TT_DESKTOPINPUT_KEYCODE_IGNORE;
}

#if defined(_DEBUG)
#define VALID_INT_CODEC(ret_val, union_codec) do {                       \
static bool recursive = false;                                           \
if(!recursive)                                                           \
{                                                                        \
    recursive = true;                                                    \
    teamtalk::AudioCodec codec_dbg1;                                     \
    AudioCodec codec_dbg2;                                               \
    Convert(ret_val, codec_dbg2);                                        \
    Convert(codec_dbg2, codec_dbg1);                                     \
    assert(ret_val == codec_dbg1);                                       \
    recursive = false;                                                   \
}                                                                        \
} while(0)

#define VALID_EXT_CODEC(ret_val, union_codec)   do {                       \
static bool recursive = false;                                             \
if(!recursive)                                                             \
{                                                                          \
    recursive = true;                                                      \
    AudioCodec codec_dbg1;                                                 \
    teamtalk::AudioCodec codec_dbg2;                                       \
    Convert(result, codec_dbg2);                                           \
    Convert(codec_dbg2, codec_dbg1);                                       \
    assert(memcmp(&(ret_val), &codec_dbg1,                                 \
                  sizeof((ret_val).nCodec) + sizeof(union_codec))  == 0);  \
    recursive = false;                                                     \
}                                                                          \
} while(0)
#else
#define VALID_INT_CODEC(ret_val, union_codec)
#define VALID_EXT_CODEC(ret_val, union_codec)
#endif


bool Convert(const AudioCodec& codec, teamtalk::AudioCodec& result)
{
    switch((teamtalk::Codec)codec.nCodec)
    {
    case teamtalk::CODEC_NO_CODEC :
        result.codec = teamtalk::CODEC_NO_CODEC;
        return true;
    case teamtalk::CODEC_SPEEX :
    {
        if(teamtalk::GetSpeexBandModeFrameSize(codec.speex.nBandmode) <= 0)
            return false;

        int fpp = 1;
        while(teamtalk::GetSpeexFramesDuration(codec.speex.nBandmode, fpp)
                < codec.speex.nTxIntervalMSec)
            fpp++;

        result.codec = teamtalk::CODEC_SPEEX;
        result.speex.bandmode = codec.speex.nBandmode;
        result.speex.quality = codec.speex.nQuality;
        result.speex.frames_per_packet = fpp;
        result.speex.sim_stereo = codec.speex.bStereoPlayback;

        VALID_INT_CODEC(result, result.speex);

        return teamtalk::ValidAudioCodec(result);
    }
    case teamtalk::CODEC_SPEEX_VBR :
    {
        if(teamtalk::GetSpeexBandModeFrameSize(codec.speex_vbr.nBandmode) <= 0)
            return false;

        int fpp = 1;
        while(teamtalk::GetSpeexFramesDuration(codec.speex_vbr.nBandmode, fpp)
                < codec.speex_vbr.nTxIntervalMSec)
            fpp++;

        result.codec = teamtalk::CODEC_SPEEX_VBR;
        result.speex_vbr.bandmode = codec.speex_vbr.nBandmode;
        result.speex_vbr.vbr_quality = codec.speex_vbr.nQuality;
        result.speex_vbr.bitrate = codec.speex_vbr.nBitRate;
        result.speex_vbr.max_bitrate = codec.speex_vbr.nMaxBitRate;
        result.speex_vbr.dtx = codec.speex_vbr.bDTX;
        result.speex_vbr.frames_per_packet = fpp;
        result.speex_vbr.sim_stereo = codec.speex_vbr.bStereoPlayback;

        VALID_INT_CODEC(result, result.speex_vbr);

        return teamtalk::ValidAudioCodec(result);
    }
#if defined(ENABLE_OPUS)
    case teamtalk::CODEC_OPUS :
    {
        result.codec = teamtalk::CODEC_OPUS;
        result.opus.samplerate = codec.opus.nSampleRate;
        result.opus.channels = codec.opus.nChannels;
        result.opus.application = codec.opus.nApplication;
        result.opus.complexity = codec.opus.nComplexity;
        result.opus.fec = codec.opus.bFEC;
        result.opus.dtx = codec.opus.bDTX;
        result.opus.bitrate = codec.opus.nBitRate;
        result.opus.vbr = codec.opus.bVBR;
        result.opus.vbr_constraint = codec.opus.bVBRConstraint;
        if (codec.opus.nFrameSizeMSec > 0)
        {
            result.opus.frame_size = OPUS_GetCbSize(codec.opus.nSampleRate, codec.opus.nFrameSizeMSec);
            if (result.opus.frame_size == 0)
                return false;

            int cbsamples = codec.opus.nSampleRate * codec.opus.nTxIntervalMSec / 1000;
            result.opus.frames_per_packet = cbsamples / result.opus.frame_size;
        }
        else
        {
            result.opus.frame_size = OPUS_GetCbSize(codec.opus.nSampleRate,
                                                    codec.opus.nTxIntervalMSec);
            result.opus.frames_per_packet = 1;
        }
        VALID_INT_CODEC(result, result.opus);

        return teamtalk::ValidAudioCodec(result);
    }
#endif

    default:
        return false;
    }
}

bool Convert(const teamtalk::AudioCodec& codec, AudioCodec& result)
{
    ZERO_STRUCT(result);

    switch(codec.codec)
    {
    case teamtalk::CODEC_NO_CODEC :
        result.nCodec = NO_CODEC;
        return true;
    case teamtalk::CODEC_SPEEX :
        result.nCodec = SPEEX_CODEC;
        result.speex.nBandmode = codec.speex.bandmode;
        result.speex.nQuality = codec.speex.quality;
        result.speex.nTxIntervalMSec = 
            teamtalk::GetSpeexFramesDuration(codec.speex.bandmode, 
            codec.speex.frames_per_packet);
        result.speex.bStereoPlayback = codec.speex.sim_stereo;
        return teamtalk::ValidAudioCodec(codec);
    case teamtalk::CODEC_SPEEX_VBR :
        result.nCodec = SPEEX_VBR_CODEC;
        result.speex_vbr.nBandmode = codec.speex_vbr.bandmode;
        result.speex_vbr.nQuality = codec.speex_vbr.vbr_quality;
        result.speex_vbr.nBitRate = codec.speex_vbr.bitrate;
        result.speex_vbr.nMaxBitRate = codec.speex_vbr.max_bitrate;
        result.speex_vbr.bDTX = codec.speex_vbr.dtx;
        result.speex_vbr.nTxIntervalMSec = 
            teamtalk::GetSpeexFramesDuration(codec.speex_vbr.bandmode, 
            codec.speex_vbr.frames_per_packet);
        result.speex_vbr.bStereoPlayback = codec.speex_vbr.sim_stereo;
        return teamtalk::ValidAudioCodec(codec);
#if defined(ENABLE_OPUS)
    case teamtalk::CODEC_OPUS :
    {
        result.nCodec = OPUS_CODEC;
        result.opus.nSampleRate = codec.opus.samplerate;
        result.opus.nChannels = codec.opus.channels;
        result.opus.nApplication = codec.opus.application;
        result.opus.nComplexity = codec.opus.complexity;
        result.opus.bFEC = codec.opus.fec;
        result.opus.bDTX = codec.opus.dtx;
        result.opus.nBitRate = codec.opus.bitrate;
        result.opus.bVBR = codec.opus.vbr;
        result.opus.bVBRConstraint = codec.opus.vbr_constraint;
        result.opus.nFrameSizeMSec = OPUS_GetCbMSec(codec.opus.samplerate,
                                                    codec.opus.frame_size);
        result.opus.nTxIntervalMSec = teamtalk::GetAudioCodecCbMillis(codec);

        VALID_EXT_CODEC(result, result.opus);
        return teamtalk::ValidAudioCodec(codec);
    }
#endif /* ENABLE_OPUS */
    default :
        return false;
    }
}

void Convert(const teamtalk::AudioConfig& audcfg, AudioConfig& result)
{
    ZERO_STRUCT(result);

    result.bEnableAGC = audcfg.enable_agc;
    result.nGainLevel = audcfg.gain_level;
}

void Convert(const AudioConfig& audcfg, teamtalk::AudioConfig& result)
{
    result.enable_agc = audcfg.bEnableAGC;
    result.gain_level = audcfg.nGainLevel;
}

void Convert(const AudioPreprocessor& audpreprocess, teamtalk::AudioPreprocessor& result)
{
    result.preprocessor = teamtalk::AudioPreprocessorType(audpreprocess.nPreprocessor);
    switch(audpreprocess.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR:
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR:
        Convert(audpreprocess.speexdsp, result.speexdsp);
        break;
    case TEAMTALK_AUDIOPREPROCESSOR :
        Convert(audpreprocess.ttpreprocessor, result.ttpreprocessor);
        break;
    }
}

void Convert(const TTAudioPreprocessor& ttpreprocess, teamtalk::TTAudioPreprocessor& result)
{
    result.gainlevel = ttpreprocess.nGainLevel;
    result.muteleft = ttpreprocess.bMuteLeftSpeaker;
    result.muteright = ttpreprocess.bMuteRightSpeaker;
}

void Convert(const teamtalk::SpeexDSP& spxdsp, SpeexDSP& result)
{
    ZERO_STRUCT(result);

    result.bEnableAGC = spxdsp.enable_agc;
    result.nGainLevel = spxdsp.agc_gainlevel;
    result.nMaxIncDBSec = spxdsp.agc_maxincdbsec;
    result.nMaxDecDBSec = spxdsp.agc_maxdecdbsec;
    result.nMaxGainDB = spxdsp.agc_maxgaindb;

    result.bEnableDenoise = spxdsp.enable_denoise;
    result.nMaxNoiseSuppressDB = spxdsp.maxnoisesuppressdb;

    result.bEnableEchoCancellation = spxdsp.enable_aec;
    result.nEchoSuppress = spxdsp.aec_suppress_level;
    result.nEchoSuppressActive = spxdsp.aec_suppress_active;
}

void Convert(const SpeexDSP& spxdsp, teamtalk::SpeexDSP& result)
{
    result.enable_agc = spxdsp.bEnableAGC;
    result.agc_gainlevel = spxdsp.nGainLevel;
    result.agc_maxincdbsec = spxdsp.nMaxIncDBSec;
    result.agc_maxdecdbsec = spxdsp.nMaxDecDBSec;
    result.agc_maxgaindb = spxdsp.nMaxGainDB;

    result.enable_denoise = spxdsp.bEnableDenoise;
    result.maxnoisesuppressdb = spxdsp.nMaxNoiseSuppressDB;

    result.enable_aec = spxdsp.bEnableEchoCancellation;
    result.aec_suppress_level = spxdsp.nEchoSuppress;
    result.aec_suppress_active = spxdsp.nEchoSuppressActive;
}

bool Convert(const teamtalk::ChannelProp& chanprop, Channel& result)
{
    ZERO_STRUCT(result);

    Convert(chanprop.audiocodec, result.audiocodec);

    result.nParentID = chanprop.parentid;
    result.nChannelID = chanprop.channelid;
    ACE_OS::strsncpy(result.szName, chanprop.name.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szTopic, chanprop.topic.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szPassword, chanprop.passwd.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szOpPassword, chanprop.oppasswd.c_str(), TT_STRLEN);
    result.bPassword = chanprop.bProtected;
    result.nMaxUsers = chanprop.maxusers;
    result.uChannelType = chanprop.chantype;
    result.nUserData = chanprop.userdata;
    result.nDiskQuota = chanprop.diskquota;
    Convert(chanprop.audiocfg, result.audiocfg);

    std::set<int> userids, tmp;
    std::set<int>::iterator ii;
    tmp = chanprop.GetTransmitUsers(teamtalk::STREAMTYPE_VOICE);
    userids.insert(tmp.begin(), tmp.end());
    tmp = chanprop.GetTransmitUsers(teamtalk::STREAMTYPE_VIDEOCAPTURE);
    userids.insert(tmp.begin(), tmp.end());
    tmp = chanprop.GetTransmitUsers(teamtalk::STREAMTYPE_DESKTOP);
    userids.insert(tmp.begin(), tmp.end());
    tmp = chanprop.GetTransmitUsers(teamtalk::STREAMTYPE_MEDIAFILE);
    userids.insert(tmp.begin(), tmp.end());
    
    ACE_OS::memset(result.transmitUsers, 0, sizeof(result.transmitUsers));
    size_t i=0;
    for(ii=userids.begin();ii!=userids.end() && i < TT_TRANSMITUSERS_MAX;ii++, i++)
    {
        result.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX] = *ii;
        if(chanprop.GetTransmitUsers(teamtalk::STREAMTYPE_VOICE).count(*ii))
            result.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] |= STREAMTYPE_VOICE;
        if(chanprop.GetTransmitUsers(teamtalk::STREAMTYPE_VIDEOCAPTURE).count(*ii))
            result.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] |= STREAMTYPE_VIDEOCAPTURE;
        if(chanprop.GetTransmitUsers(teamtalk::STREAMTYPE_DESKTOP).count(*ii))
            result.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] |= STREAMTYPE_DESKTOP;
        if(chanprop.GetTransmitUsers(teamtalk::STREAMTYPE_MEDIAFILE).count(*ii))
            result.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] |= STREAMTYPE_MEDIAFILE;
    }

    for(i=0;i<TT_TRANSMITQUEUE_MAX;i++)
    {
        if(i < chanprop.transmitqueue.size())
            result.transmitUsersQueue[i] = chanprop.transmitqueue[i];
        else
            result.transmitUsersQueue[i] = 0;
    }

    return true;
}

bool Convert(const Channel& channel, teamtalk::ChannelProp& chanprop)
{
    if(!Convert(channel.audiocodec, chanprop.audiocodec))
        return false;

    chanprop.channelid = channel.nChannelID;
    chanprop.parentid = channel.nParentID;
    chanprop.name = channel.szName;
    chanprop.chantype = channel.uChannelType;
    chanprop.userdata = channel.nUserData;
    chanprop.diskquota = channel.nDiskQuota;
    chanprop.maxusers = channel.nMaxUsers;
    chanprop.oppasswd = channel.szOpPassword;
    chanprop.passwd = channel.szPassword;
    chanprop.topic = channel.szTopic;
    Convert(channel.audiocfg, chanprop.audiocfg);

    for(int i=0;i<TT_TRANSMITUSERS_MAX && channel.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX];i++)
    {
        int userid = channel.transmitUsers[i][TT_TRANSMITUSERS_USERID_INDEX];
        if(channel.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] & STREAMTYPE_VOICE)
            chanprop.transmitusers[teamtalk::STREAMTYPE_VOICE].insert(userid);
        if(channel.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] & STREAMTYPE_VIDEOCAPTURE)
            chanprop.transmitusers[teamtalk::STREAMTYPE_VIDEOCAPTURE].insert(userid);
        if(channel.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] & STREAMTYPE_DESKTOP)
            chanprop.transmitusers[teamtalk::STREAMTYPE_DESKTOP].insert(userid);
        if(channel.transmitUsers[i][TT_TRANSMITUSERS_STREAMTYPE_INDEX] & (STREAMTYPE_MEDIAFILE))
            chanprop.transmitusers[teamtalk::STREAMTYPE_MEDIAFILE].insert(userid);
    }

    for(int i=0;i<TT_TRANSMITQUEUE_MAX;i++)
    {
        if(channel.transmitUsersQueue[i])
            chanprop.transmitqueue.push_back(channel.transmitUsersQueue[i]);
    }

    return true;
}

void Convert(const teamtalk::User& srcuser, User& result)
{
    result.nUserID = srcuser.GetUserID();
    ACE_OS::strsncpy(result.szNickname, srcuser.GetNickname().c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szUsername, srcuser.GetUsername().c_str(), TT_STRLEN);
    result.nStatusMode = srcuser.GetStatusMode();
    ACE_OS::strsncpy(result.szStatusMsg, srcuser.GetStatusMessage().c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szIPAddress, srcuser.GetIpAddress().c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szClientName, srcuser.GetClientName().c_str(), TT_STRLEN);
    strings_t tokens = tokenize(srcuser.GetClientVersion(), ACE_TEXT("."));
    result.uVersion = 0;
    int shift = 16;
    while(tokens.size() && shift >= 0)
    {
        result.uVersion |= string2i(tokens[0]) << shift;
        tokens.erase(tokens.begin());
        shift -= 8;
    }
    result.nUserData = srcuser.GetUserData();
    result.uUserType = srcuser.GetUserType();
}

void Convert(const teamtalk::ClientUser& clientuser, User& result)
{
    ZERO_STRUCT(result);

    Convert(static_cast<const teamtalk::User&>(clientuser), result);

    teamtalk::clientchannel_t channel = clientuser.GetChannel();
    if(channel)
        result.nChannelID = channel->GetChannelID();
    else
        result.nChannelID = 0;

    result.uLocalSubscriptions = clientuser.GetLocalSubscriptions();
    result.uPeerSubscriptions = clientuser.GetPeerSubscriptions();

    result.uUserState = clientuser.IsAudioActive(teamtalk::STREAMTYPE_VOICE)?
                                               USERSTATE_VOICE : USERSTATE_NONE;
    if(clientuser.IsMute(teamtalk::STREAMTYPE_VOICE))
        result.uUserState |= USERSTATE_MUTE_VOICE;
    if(clientuser.IsMute(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO))
        result.uUserState |= USERSTATE_MUTE_MEDIAFILE;
    if (clientuser.GetDesktopSession())
        result.uUserState |= USERSTATE_DESKTOP;
    teamtalk::VideoCodec vidcodec;
    if(clientuser.GetVideoCaptureCodec(vidcodec))
        result.uUserState |= USERSTATE_VIDEOCAPTURE;
    if(clientuser.GetVideoFileCodec(vidcodec))
        result.uUserState |= USERSTATE_MEDIAFILE_VIDEO;
    if(clientuser.IsAudioActive(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO))
        result.uUserState |= USERSTATE_MEDIAFILE_AUDIO;
    ACE_OS::strsncpy(result.szMediaStorageDir, clientuser.GetAudioFolder().c_str(), TT_STRLEN);
    result.nVolumeVoice = clientuser.GetVolume(teamtalk::STREAMTYPE_VOICE);
    result.nVolumeMediaFile = clientuser.GetVolume(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO);
    result.nStoppedDelayVoice = clientuser.GetPlaybackStoppedDelay(teamtalk::STREAMTYPE_VOICE);
    result.nStoppedDelayMediaFile = clientuser.GetPlaybackStoppedDelay(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO);
    float x,y,z;
    clientuser.GetPosition(teamtalk::STREAMTYPE_VOICE, x, y, z);
    result.soundPositionVoice[0] = x;
    result.soundPositionVoice[1] = y;
    result.soundPositionVoice[2] = z;
    clientuser.GetPosition(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO, x, y, z);
    result.soundPositionMediaFile[0] = x;
    result.soundPositionMediaFile[1] = y;
    result.soundPositionMediaFile[2] = z;
    bool l,r;
    clientuser.GetStereo(teamtalk::STREAMTYPE_VOICE, l, r);
    result.stereoPlaybackVoice[0] = l;
    result.stereoPlaybackVoice[1] = r;
    clientuser.GetStereo(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO, l, r);
    result.stereoPlaybackMediaFile[0] = l;
    result.stereoPlaybackMediaFile[1] = r;
    result.nBufferMSecVoice = clientuser.GetAudioStreamBufferSize(teamtalk::STREAMTYPE_VOICE);
    result.nBufferMSecMediaFile = clientuser.GetAudioStreamBufferSize(teamtalk::STREAMTYPE_MEDIAFILE_AUDIO);
}

#if defined(ENABLE_TEAMTALKPRO)
void Convert(const teamtalk::ServerUser& serveruser, User& result)
{
    ZERO_STRUCT(result);

    Convert(static_cast<const teamtalk::User&>(serveruser), result);
    teamtalk::serverchannel_t channel = serveruser.GetChannel();
    if(channel)
        result.nChannelID = channel->GetChannelID();
    else
        result.nChannelID = 0;
}
#endif

void Convert(const VideoCodec& vidcodec, teamtalk::VideoCodec& result)
{
    switch(vidcodec.nCodec)
    {
    case SPEEX_CODEC :
    case SPEEX_VBR_CODEC :
    case OPUS_CODEC :
    case NO_CODEC :
        break;
    case WEBM_VP8_CODEC :
        result.codec = teamtalk::CODEC_WEBM_VP8;
        result.webm_vp8.rc_target_bitrate = vidcodec.webm_vp8.nRcTargetBitrate;
        result.webm_vp8.encode_deadline = vidcodec.webm_vp8.nEncodeDeadline;
        break;
    }
    assert(result.codec == (teamtalk::Codec)vidcodec.nCodec);
}

void Convert(const teamtalk::UserAccount& useraccount, UserAccount& result)
{
    ZERO_STRUCT(result);

    ACE_OS::strsncpy(result.szUsername, useraccount.username.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szPassword, useraccount.passwd.c_str(), TT_STRLEN);
    result.uUserType = useraccount.usertype;
    result.uUserRights = useraccount.userrights;
    result.nUserData = useraccount.userdata;
    ACE_OS::strsncpy(result.szNote, useraccount.note.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szInitChannel, useraccount.init_channel.c_str(), TT_STRLEN);
    Convert(useraccount.auto_op_channels, result.autoOperatorChannels, TT_CHANNELS_OPERATOR_MAX);
    result.nAudioCodecBpsLimit = useraccount.audiobpslimit;
    result.abusePrevent.nCommandsLimit = useraccount.abuse.n_cmds;
    result.abusePrevent.nCommandsIntervalMSec = useraccount.abuse.cmd_msec;
}

void Convert(const UserAccount& useraccount, teamtalk::UserAccount& result)
{
    result.username = useraccount.szUsername;
    result.passwd = useraccount.szPassword;
    result.usertype = useraccount.uUserType;
    result.userrights = useraccount.uUserRights;
    result.userdata = useraccount.nUserData;
    result.note = useraccount.szNote;
    result.init_channel = useraccount.szInitChannel;
    result.audiobpslimit = useraccount.nAudioCodecBpsLimit;
    Convert(useraccount.autoOperatorChannels, TT_CHANNELS_OPERATOR_MAX, result.auto_op_channels);
    result.abuse.n_cmds = useraccount.abusePrevent.nCommandsLimit;
    result.abuse.cmd_msec = useraccount.abusePrevent.nCommandsIntervalMSec;
}

void Convert(const teamtalk::ServerProperties& srvprop, ServerProperties& result)
{
    result.nMaxUsers = srvprop.maxusers;
    result.nMaxLoginAttempts = srvprop.maxloginattempts;
    result.nMaxLoginsPerIPAddress = srvprop.max_logins_per_ipaddr;
    result.nUserTimeout = srvprop.usertimeout;
    ACE_OS::strsncpy(result.szServerName, srvprop.servername.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szMOTD, srvprop.motd.c_str(), TT_STRLEN);
    result.nMaxVoiceTxPerSecond = srvprop.voicetxlimit;
    result.nMaxVideoCaptureTxPerSecond = srvprop.videotxlimit;
    result.nMaxMediaFileTxPerSecond = srvprop.mediafiletxlimit;
    result.nMaxDesktopTxPerSecond = srvprop.desktoptxlimit;
    result.nMaxTotalTxPerSecond = srvprop.totaltxlimit;
    result.bAutoSave = srvprop.autosave;
    result.nMaxUsers = srvprop.maxusers;
    result.nUserTimeout = srvprop.usertimeout;
    result.nLoginDelayMSec = srvprop.logindelay;
    ACE_OS::strsncpy(result.szServerVersion, srvprop.version.c_str(), TT_STRLEN);
}

void Convert(const teamtalk::ServerInfo& srvprop, ServerProperties& result)
{
    ZERO_STRUCT(result);
    Convert(static_cast<const teamtalk::ServerProperties&>(srvprop), result);
    ACE_OS::strsncpy(result.szMOTDRaw, srvprop.motd_raw.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szServerProtocolVersion, srvprop.protocol.c_str(), TT_STRLEN);
    if (srvprop.hostaddrs.size())
    {
        result.nTcpPort = srvprop.hostaddrs[0].get_port_number();
        result.nUdpPort = srvprop.udpaddr.get_port_number();
    }
    ACE_OS::strsncpy(result.szAccessToken, srvprop.accesstoken.c_str(), TT_STRLEN);
}

#if defined(ENABLE_TEAMTALKPRO)
void Convert(const teamtalk::ServerSettings& srvprop, ServerProperties& result)
{
    ZERO_STRUCT(result);
    Convert(static_cast<const teamtalk::ServerProperties&>(srvprop), result);
    ACE_OS::strsncpy(result.szServerProtocolVersion, TEAMTALK_PROTOCOL_VERSION, TT_STRLEN);

    if (srvprop.tcpaddrs.size())
        result.nTcpPort = srvprop.tcpaddrs[0].get_port_number();
    if (srvprop.udpaddrs.size())
        result.nUdpPort = srvprop.udpaddrs[0].get_port_number();
}
#endif

void Convert(const ServerProperties& srvprop, teamtalk::ServerProperties& result)
{
    result.servername = srvprop.szServerName;
    result.motd = srvprop.szMOTD;
    result.maxusers = srvprop.nMaxUsers;
    result.maxloginattempts = srvprop.nMaxLoginAttempts;
    result.max_logins_per_ipaddr = srvprop.nMaxLoginsPerIPAddress;
    result.voicetxlimit = srvprop.nMaxVoiceTxPerSecond;
    result.videotxlimit = srvprop.nMaxVideoCaptureTxPerSecond;
    result.mediafiletxlimit = srvprop.nMaxMediaFileTxPerSecond;
    result.desktoptxlimit = srvprop.nMaxDesktopTxPerSecond;
    result.totaltxlimit = srvprop.nMaxTotalTxPerSecond;
    result.autosave = srvprop.bAutoSave;
    result.usertimeout = srvprop.nUserTimeout;
    result.logindelay = srvprop.nLoginDelayMSec;
}

void Convert(const ServerProperties& srvprop, teamtalk::ServerInfo& result)
{
    Convert(srvprop, static_cast<teamtalk::ServerProperties&>(result));

    if (result.hostaddrs.size())
    {
        result.hostaddrs[0].set_port_number(srvprop.nTcpPort);
        result.udpaddr.set_port_number(srvprop.nUdpPort);
    }
    result.motd_raw = srvprop.szMOTDRaw;
    result.accesstoken = srvprop.szAccessToken;
}

#if defined(ENABLE_TEAMTALKPRO)
void Convert(const ServerProperties& srvprop, teamtalk::ServerSettings& result)
{
    Convert(srvprop, static_cast<teamtalk::ServerProperties&>(result));
    
    for (auto& addr : result.tcpaddrs)
        addr.set_port_number(srvprop.nTcpPort);
    for (auto& addr : result.udpaddrs)
        addr.set_port_number(srvprop.nUdpPort);
}
#endif

ClientErrorMsg& Convert(const teamtalk::ErrorMsg& cmderr, ClientErrorMsg& result)
{
    ZERO_STRUCT(result);

    ACE_OS::strsncpy(result.szErrorMsg, cmderr.errmsg.c_str(), TT_STRLEN);
    result.nErrorNo = cmderr.errorno;
    return result;
}

teamtalk::ErrorMsg& Convert(const ClientErrorMsg& cmderr, teamtalk::ErrorMsg& result)
{
    result.errmsg = cmderr.szErrorMsg;
    result.errorno = cmderr.nErrorNo;
    return result;
}

void Convert(const teamtalk::BannedUser& banuser, BannedUser& result)
{
    ZERO_STRUCT(result);

    ACE_OS::strsncpy(result.szChannelPath, banuser.chanpath.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szIPAddress, banuser.ipaddr.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szNickname, banuser.nickname.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szUsername, banuser.username.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szBanTime, teamtalk::DateToString( banuser.bantime ).c_str(), TT_STRLEN);
    result.uBanTypes = BanTypes(banuser.bantype);
}

void Convert(const BannedUser& banuser, teamtalk::BannedUser& result)
{
    result.bantype = teamtalk::BanTypes(banuser.uBanTypes);
    result.chanpath = banuser.szChannelPath;
    result.ipaddr = banuser.szIPAddress;
    result.nickname = banuser.szNickname;
    result.username = banuser.szUsername;
}

void Convert(const teamtalk::FileTransfer& transfer, FileTransfer& result)
{
    ZERO_STRUCT(result);

    result.nStatus = (FileTransferStatus)transfer.status;
    ACE_OS::strsncpy(result.szLocalFilePath, transfer.localfile.c_str(), TT_STRLEN);
    result.nTransferID = transfer.transferid;
    result.nFileSize = transfer.filesize;
    result.nTransferred = transfer.transferred;
    result.bInbound = transfer.inbound;
    ACE_OS::strsncpy(result.szRemoteFileName, transfer.filename.c_str(), TT_STRLEN);
    result.nChannelID = transfer.channelid;
}

void Convert(const teamtalk::RemoteFile& remotefile, RemoteFile& result)
{
    ZERO_STRUCT(result);

    ACE_OS::strsncpy(result.szFileName, remotefile.filename.c_str(), TT_STRLEN);
    ACE_OS::strsncpy(result.szUsername,  remotefile.username.c_str(), TT_STRLEN);
    result.nFileID = remotefile.fileid;
    result.nFileSize = remotefile.filesize;
    result.nChannelID = remotefile.channelid;
}

void Convert(const RemoteFile& remotefile, const TTCHAR* szPath, teamtalk::RemoteFile& result)
{
    result.channelid = remotefile.nChannelID;
    result.fileid = remotefile.nFileID;
    result.filename = remotefile.szFileName;
    result.filesize = remotefile.nFileSize;
    result.internalname = szPath;
    result.username = remotefile.szUsername;
}

void Convert(const teamtalk::ServerStats& stats, ServerStatistics& result)
{
    ZERO_STRUCT(result);

    result.nTotalBytesTX = stats.total_bytessent;
    result.nTotalBytesRX = stats.total_bytesreceived;
    result.nVoiceBytesTX = stats.voice_bytessent;
    result.nVoiceBytesRX = stats.voice_bytesreceived;
    result.nVideoCaptureBytesTX = stats.vidcap_bytessent;
    result.nVideoCaptureBytesRX = stats.vidcap_bytesreceived;
    result.nMediaFileBytesTX = stats.mediafile_bytessent;
    result.nMediaFileBytesRX = stats.mediafile_bytesreceived;
    result.nDesktopBytesTX = stats.desktop_bytessent;
    result.nDesktopBytesRX = stats.desktop_bytesreceived;
    result.nUsersServed = stats.usersservered;
    result.nUsersPeak = stats.userspeak;
    result.nFilesTx = stats.files_bytessent;
    result.nFilesRx = stats.files_bytesreceived;
    ACE_UINT64 msec = 0;
    stats.starttime.msec(msec);
    result.nUptimeMSec = msec;
}

void Convert(const teamtalk::TextMessage& txtmsg, TextMessage& result)
{
    ZERO_STRUCT(result);

    result.nMsgType = (TextMsgType)txtmsg.msgType;
    ACE_OS::strsncpy(result.szMessage, txtmsg.content.c_str(), TT_STRLEN);
    result.nFromUserID = txtmsg.from_userid;
    ACE_OS::strsncpy(result.szFromUsername, txtmsg.from_username.c_str(), TT_STRLEN);
    result.nToUserID = txtmsg.to_userid;
    result.nChannelID = txtmsg.channelid;
}

void Convert(const TextMessage& txtmsg, teamtalk::TextMessage& result)
{
    result.msgType = (teamtalk::MsgType)txtmsg.nMsgType;
    result.from_userid = txtmsg.nFromUserID;
    result.to_userid = txtmsg.nToUserID;
    result.content = txtmsg.szMessage;
    result.channelid = txtmsg.nChannelID;
}

void Convert(const MediaFileProp& mediaprop, MediaFileInfo& result)
{
    ZERO_STRUCT(result);

    if(mediaprop.audio.IsValid())
    {
        result.audioFmt.nAudioFmt = AFF_WAVE_FORMAT;
        result.audioFmt.nChannels = mediaprop.audio.channels;
        result.audioFmt.nSampleRate = mediaprop.audio.samplerate;
    }
    else
        result.audioFmt.nAudioFmt = AFF_NONE;

    if(mediaprop.video.IsValid())
    {
        result.videoFmt.picFourCC = FOURCC_RGB32;
        result.videoFmt.nWidth = mediaprop.video.width;
        result.videoFmt.nHeight = mediaprop.video.height;
        result.videoFmt.nFPS_Numerator = mediaprop.video.fps_numerator;
        result.videoFmt.nFPS_Denominator = mediaprop.video.fps_denominator;
    }
    else
        result.videoFmt.picFourCC = FOURCC_NONE;
    result.uDurationMSec = mediaprop.duration_ms;
    result.uElapsedMSec = mediaprop.elapsed_ms;
    ACE_OS::strsncpy(result.szFileName, mediaprop.filename.c_str(), TT_STRLEN);
}

void Convert(teamtalk::MediaFileStatus status, const teamtalk::VoiceLogFile& vlog,
             MediaFileInfo& result)
{
    ZERO_STRUCT(result);

    result.nStatus = (MediaFileStatus)status;
    result.audioFmt.nAudioFmt = (AudioFileFormat)vlog.aff;
    result.audioFmt.nChannels = vlog.channels;
    result.audioFmt.nSampleRate = vlog.samplerate;
    result.uDurationMSec = vlog.duration;
    result.uElapsedMSec = 0;
    ACE_OS::strsncpy(result.szFileName, vlog.filename.c_str(), TT_STRLEN);
    ACE_OS::memset(&result.videoFmt, 0, sizeof(result.videoFmt));
}

void Convert(const AudioInputStatus& ais, AudioInputProgress& result)
{
    ZERO_STRUCT(result);
    result.uElapsedMSec = ais.elapsed_msec;
    result.uQueueMSec = ais.queueduration_msec;
    result.nStreamID = ais.streamid;
}

void Convert(const media::VideoFrame& imgframe, VideoFrame& result)
{
    ZERO_STRUCT(result);

    TTASSERT(RGB32_BYTES(imgframe.width, imgframe.height) == imgframe.frame_length);
    result.frameBuffer = imgframe.frame;
    result.nWidth = imgframe.width;
    result.nHeight = imgframe.height;
    result.nStreamID = imgframe.stream_id;
    result.bKeyFrame = imgframe.key_frame;
    result.nFrameBufferSize = imgframe.frame_length;
}

void Convert(const VideoFormat& vidfmt, media::VideoFormat& result)
{
    result.width = vidfmt.nWidth;
    result.height = vidfmt.nHeight;
    result.fps_numerator = vidfmt.nFPS_Numerator;
    result.fps_denominator = vidfmt.nFPS_Denominator;
    result.fourcc = (media::FourCC)vidfmt.picFourCC;
}

void Convert(const teamtalk::ClientUserStats& stats, UserStatistics& result)
{
    ZERO_STRUCT(result);

    result.nVoicePacketsRecv = stats.voicepackets_recv;
    result.nVoicePacketsLost = stats.voicepackets_lost;
    result.nVideoCapturePacketsRecv = stats.vidcappackets_recv;
    result.nVideoCaptureFramesRecv = stats.vidcapframes_recv;
    result.nVideoCaptureFramesLost = stats.vidcapframes_lost;
    result.nVideoCaptureFramesDropped = stats.vidcapframes_dropped;
    result.nMediaFileAudioPacketsRecv = stats.mediafile_audiopackets_recv;
    result.nMediaFileAudioPacketsLost = stats.mediafile_audiopackets_lost;
    result.nMediaFileVideoPacketsRecv = stats.mediafile_video_packets_recv;
    result.nMediaFileVideoFramesRecv = stats.mediafile_video_frames_recv;
    result.nMediaFileVideoFramesLost = stats.mediafile_video_frames_lost;
    result.nMediaFileVideoFramesDropped = stats.mediafile_video_frames_dropped;
}

void Convert(const teamtalk::ClientStats& stats, ClientStatistics& result)
{
    ZERO_STRUCT(result);

    result.nUdpBytesRecv = stats.udpbytes_recv;
    result.nUdpBytesSent = stats.udpbytes_sent;
    result.nVoiceBytesRecv = stats.voicebytes_recv;
    result.nVoiceBytesSent = stats.voicebytes_sent;
    result.nVideoCaptureBytesRecv = stats.vidcapbytes_recv;
    result.nVideoCaptureBytesSent = stats.vidcapbytes_sent;
    result.nMediaFileAudioBytesRecv = stats.mediafile_audio_bytes_recv;
    result.nMediaFileAudioBytesSent = stats.mediafile_audio_bytes_sent;
    result.nMediaFileVideoBytesRecv = stats.mediafile_video_bytes_recv;
    result.nMediaFileVideoBytesSent = stats.mediafile_video_bytes_sent;
    result.nDesktopBytesRecv = stats.desktopbytes_recv;
    result.nDesktopBytesSent = stats.desktopbytes_sent;
    result.nTcpPingTimeMs = stats.tcpping_time;
    result.nUdpPingTimeMs = stats.udpping_time;
    result.nTcpServerSilenceSec = stats.tcp_silence_sec;
    result.nUdpServerSilenceSec = stats.udp_silence_sec;
}

void Convert(const ClientKeepAlive& ka, teamtalk::ClientKeepAlive& result)
{
    result.connection_lost.msec(ka.nConnectionLostMSec);
    result.tcp_keepalive_interval.msec(ka.nTcpKeepAliveIntervalMSec);
    result.udp_keepalive_interval.msec(ka.nUdpKeepAliveIntervalMSec);
    result.udp_keepalive_rtx.msec(ka.nUdpKeepAliveRTXMSec);
    result.udp_connect_interval.msec(ka.nUdpConnectRTXMSec);
    result.udp_connect_timeout.msec(ka.nUdpConnectTimeoutMSec);
}

void Convert(const teamtalk::ClientKeepAlive& ka, ClientKeepAlive& result)
{
    result.nConnectionLostMSec = ka.connection_lost.msec();
    result.nTcpKeepAliveIntervalMSec = ka.tcp_keepalive_interval.msec();
    result.nUdpKeepAliveIntervalMSec = ka.udp_keepalive_interval.msec();
    result.nUdpKeepAliveRTXMSec = ka.udp_keepalive_rtx.msec();
    result.nUdpConnectRTXMSec = ka.udp_connect_interval.msec();
    result.nUdpConnectTimeoutMSec = ka.udp_connect_timeout.msec();
}

void Convert(const teamtalk::DesktopInput& input, DesktopInput& result)
{
    ZERO_STRUCT(result);

    result.uMousePosX = input.x;
    result.uMousePosY = input.y;
    result.uKeyCode = input.keycode;
    result.uKeyState = input.keystate;
}

void Convert(const DesktopInput& input, teamtalk::DesktopInput& result)
{
    result.x = input.uMousePosX;
    result.y = input.uMousePosY;
    result.keycode = input.uKeyCode;
    result.keystate = input.uKeyState;
}


void Convert(const std::set<int>& intset, int* int_array, int max_elements)
{
    std::set<int>::const_iterator ite = intset.begin();
    for(int i=0;i<max_elements;i++)
    {
        if(ite != intset.end())
        {
            int_array[i] = *ite;
            ite++;
        }
        else
            int_array[i] = 0;
    }
}

void Convert(const int* int_array, int max_elements, std::set<int>& intset)
{
    int i=0;
    while(i<max_elements && int_array[i]>0)
    {
        intset.insert(int_array[i]);
        i++;
    }
}
