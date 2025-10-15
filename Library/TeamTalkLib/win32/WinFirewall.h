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

#ifndef WINFIREWALL_H
#define WINFIREWALL_H

#include <ace/SString.h>

#include <netfw.h>

class WinFirewall
{
public:
    WinFirewall(const WinFirewall&) = delete;
    WinFirewall(bool acquire_admin);
    ~WinFirewall();

    static bool HasUAE();

    bool IsFirewallOn();

    bool EnableFirewall(bool enable);

    bool IsApplicationFirewalled(const ACE_TString& exefile);

    bool AddException(const ACE_TString& exefile,
                      const ACE_TString& name);
    bool RemoveException(const ACE_TString& exefile);

private:
    bool OpenFirewall();
    INetFwProfile* m_fw;
};

#endif
