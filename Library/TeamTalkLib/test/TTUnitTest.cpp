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

#include <ace/ACE.h>

#include "TTUnitTest.h"

#include <myace/MyACE.h>

bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, std::function<bool(TTMessage)> pred, TTMessage& outmsg /*= TTMessage()*/, int timeout /*= DEFWAIT*/)
{
    auto start = GETTIMESTAMP();
    while(GETTIMESTAMP() < start + timeout)
    {
        INT32 waitMsec = 10;
        if(TT_GetMessage(ttClient, &outmsg, &waitMsec) &&
            outmsg.nClientEvent == ttevent &&
            pred(outmsg))
            return true;
    }
    return false;
}

bool WaitForEvent(TTInstance* ttClient, ClientEvent ttevent, TTMessage& outmsg /*= TTMessage()*/, int timeout /*= DEFWAIT*/)
{
    return WaitForEvent(ttClient, ttevent, [](TTMessage) { return true; }, outmsg, timeout);
}

bool WaitForCmdSuccess(TTInstance* ttClient, int cmdid, TTMessage& outmsg /*= TTMessage()*/, int timeout /*= DEFWAIT*/)
{
    return WaitForEvent(ttClient, CLIENTEVENT_CMD_SUCCESS, [cmdid](TTMessage msg) {
        return msg.nSource == cmdid;
    }, outmsg, timeout);
}

bool WaitForCmdComplete(TTInstance* ttClient, int cmdid, TTMessage& outmsg /*= TTMessage()*/, int timeout /*= DEFWAIT*/)
{
    return WaitForEvent(ttClient, CLIENTEVENT_CMD_PROCESSING, [cmdid](TTMessage msg) {
        return msg.nSource == cmdid && !msg.bActive;
    }, outmsg, timeout);
}