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


#include "stdafx.h"
#include "Helper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

#if defined(UNICODE) || defined(_UNICODE)
CString CONVERTSTR(const char* str)
{
	ASSERT(str);
	int size = strlen(str)+1;
	wchar_t* buff = new wchar_t[size];
	buff[size-1] = 0;
	int ret = MultiByteToWideChar(CP_UTF8, 0, str, -1, buff, size);
	ASSERT(ret>0);

	CString s = buff;
	delete [] buff;
	return s;
}

std::string CONVERTSTR(const wchar_t* str)
{
	ASSERT(str);
	int size = 4*wcslen(str)+1;
	char* buff = new char[size];
	buff[size-1] = 0;
	int ret = WideCharToMultiByte(CP_UTF8, 0, str, -1, buff, size, NULL, NULL);
	ASSERT(ret>0);
	string s(buff);
	delete [] buff;
	return s;
}

#endif
