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

#include "ttassert.h"

#include <string>
#include <sstream>
#include "Log.h"
#include <myace/MyACE.h>

using namespace std;

#if defined(UNICODE)
#define ostinrgstream wostringstream
#endif

void tt_assert(const char* assertion, const char* file, int line)
{
    ostringstream os;
    os << "Failed assertion ";
    os << assertion;
    os << " in file ";
    os << file;
    os << " at line ";
    os << line << endl;

    TT_ERROR(LocalToUnicode(os.str().c_str()));

    // Here you can print info on how to report bug
    // ...

    // Here you flush streams and save date and close files.
    // You'll need a way of accessing non-global variables,
    // but there are many ways around this.
    // ...
}
