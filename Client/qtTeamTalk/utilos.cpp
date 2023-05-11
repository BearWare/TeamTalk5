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

#include "utilos.h"

#include <QtGlobal>

#if defined(Q_OS_WIN32)

#include <windows.h>
bool isComputerIdle(int idle_secs)
{
    LASTINPUTINFO info;
    info.cbSize = sizeof(LASTINPUTINFO);
    if( GetLastInputInfo(&info))
        return (::GetTickCount() - info.dwTime) / 1000 >= (UINT)idle_secs;
    else
        return false;
}

#elif defined(Q_OS_DARWIN)

#include <IOKit/IOKitLib.h>
#include <Carbon/Carbon.h>

bool isComputerIdle(int idle_secs)
{
    int64_t os_idle_secs = 0;
    io_iterator_t iter = 0;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault,
                                     IOServiceMatching("IOHIDSystem"),
                                     &iter) == KERN_SUCCESS)
    {
        io_registry_entry_t entry = IOIteratorNext(iter);
        if (entry)
        {
            CFMutableDictionaryRef dict = nullptr;
            if (IORegistryEntryCreateCFProperties(entry, &dict,
                                                  kCFAllocatorDefault,
                                                  0) == KERN_SUCCESS)
            {
                CFNumberRef obj = static_cast<CFNumberRef>
                    (CFDictionaryGetValue(dict,
                                          CFSTR("HIDIdleTime")));
                if (obj)
                {
                    int64_t nanoseconds = 0;
                    if (CFNumberGetValue(obj, kCFNumberSInt64Type,
                                         &nanoseconds))
                    {
                        // Divide by 10^9 to convert from nanoseconds to seconds.
                        os_idle_secs = (nanoseconds >> 30);
                    }
                }
            }
            IOObjectRelease(entry);
        }
        IOObjectRelease(iter);
    }
    return (int)os_idle_secs > idle_secs;
}
#else
bool isComputerIdle(int /*idle_secs*/)
{
    return false;
}
#endif
