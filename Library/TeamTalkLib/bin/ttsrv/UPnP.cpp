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

#include "UPnP.h"
#include "teamtalk/Log.h"

#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

#include <array>
#include <cstring>
#include <string>

static struct UPNPUrls s_urls = {};
static struct IGDdatas s_data = {};
std::string lanIPAddr;

bool UPnP_AddPortMapping(uint16_t tcpport, uint16_t udpport, std::string& externalIP)
{
    int error = 0;
    struct UPNPDev* devlist = upnpDiscover(2000, nullptr, nullptr, 0, 0, 2, &error);
    if (!devlist)
    {
        TT_LOG(ACE_TEXT("UPnP: No IGD devices found."));
        return false;
    }

    std::array<char,40> s_lanaddr;
#if MINIUPNPC_API_VERSION >= 18
    std::array<char, 40> wanaddr;
    int r = UPNP_GetValidIGD(devlist, &s_urls, &s_data,
                              s_lanaddr.data(), s_lanaddr.size(),
                              wanaddr.data(), wanaddr.size());
#else
    int r = UPNP_GetValidIGD(devlist, &s_urls, &s_data,
                             s_lanaddr.data(), s_lanaddr.size());
#endif
    freeUPNPDevlist(devlist);

    switch (r)
    {
    case UPNP_CONNECTED_IGD :
    case UPNP_PRIVATEIP_IGD :
        break;
    default :
        TT_LOG(ACE_TEXT("UPnP: No valid IGD found."));
        return false;
    }

    std::array<char, 16> extIP;
    switch (UPNP_GetExternalIPAddress(s_urls.controlURL,
                                      s_data.first.servicetype, extIP.data()))
    {
    case UPNPCOMMAND_SUCCESS:
        externalIP = extIP.data();
        break;
    default :
        TT_LOG(ACE_TEXT("UPnP: Failed to get the external IP-address."));
        return false;
    }

    lanIPAddr = s_lanaddr.data();

    return UPnP_RenewPortMapping(tcpport, udpport);
}

bool UPnP_RenewPortMapping(uint16_t tcpport, uint16_t udpport)
{
    if (lanIPAddr.empty())
        return false;

    std::string const tcp_str = std::to_string(tcpport);
    int ret = UPNP_AddPortMapping(s_urls.controlURL,
        s_data.first.servicetype,
        tcp_str.c_str(), tcp_str.c_str(), lanIPAddr.c_str(),
        "TeamTalk Server", "TCP", "", "0");
    if (ret != UPNPCOMMAND_SUCCESS)
    {
        TT_LOG(ACE_TEXT("UPnP: Failed to add TCP port mapping."));
        return false;
    }

    std::string const udp_str = std::to_string(udpport);
    ret = UPNP_AddPortMapping(s_urls.controlURL,
        s_data.first.servicetype,
        udp_str.c_str(), udp_str.c_str(), lanIPAddr.c_str(),
        "TeamTalk Server", "UDP", "", "0");
    if (ret != UPNPCOMMAND_SUCCESS)
    {
        TT_LOG(ACE_TEXT("UPnP: Failed to add UDP port mapping."));
        return false;
    }

    return true;
}

void UPnP_RemovePortMapping(uint16_t tcpport, uint16_t udpport)
{
    if (lanIPAddr.empty())
        return;

    std::string const tcp_str = std::to_string(tcpport);
    std::string const udp_str = std::to_string(udpport);

    UPNP_DeletePortMapping(s_urls.controlURL,
        s_data.first.servicetype, tcp_str.c_str(), "TCP", "");
    UPNP_DeletePortMapping(s_urls.controlURL,
        s_data.first.servicetype, udp_str.c_str(), "UDP", "");

    FreeUPNPUrls(&s_urls);
    lanIPAddr.clear();

    TT_LOG(ACE_TEXT("UPnP: Port mappings removed."));
}
