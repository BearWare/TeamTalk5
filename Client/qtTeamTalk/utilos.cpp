/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
#elif defined(Q_OS_LINUX)

#include <X11/extensions/scrnsaver.h>
#include <X11/Xlib.h>

bool isComputerIdle(int idle_secs)
{
    Display* dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        return false;
    }

    XScreenSaverInfo *info = XScreenSaverAllocInfo();
    if (!info)
    {
        XCloseDisplay(dpy);
        return false;
    }

    XScreenSaverQueryInfo(dpy, DefaultRootWindow(dpy), info);
    int idle_time = info->idle / 1000;

    XFree(info);
    XCloseDisplay(dpy);

    return idle_time >= idle_secs;
}

#else
bool isComputerIdle(int /*idle_secs*/)
{
    return false;
}
#endif
