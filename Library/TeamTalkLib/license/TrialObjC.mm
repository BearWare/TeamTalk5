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

#include "Trial.h"

#import <Foundation/Foundation.h>

ACE_TString GetProcessName()
{
    ACE_TCHAR buff[512] = ACE_TEXT("Unknown");

    NSBundle *bundle = [NSBundle mainBundle];       
    NSDictionary *info = [bundle infoDictionary];
    NSString *prodName = [info objectForKey:@"CFBundleDisplayName"];

    if(!prodName)
        prodName = [info objectForKey:@"CFBundleName"];
    if(!prodName)
        prodName = [info objectForKey:@"CFBundleIdentifier"];
    if(!prodName)
        prodName = [info objectForKey:@"CFBundleExecutable"];

    if(prodName)
    {
        const char *c = [prodName UTF8String];
        if(c)
            ACE_OS::strncpy(buff, c, 512);
    }

    return ACE_TString(buff);
}
