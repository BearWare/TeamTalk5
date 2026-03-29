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

#include "Convert.h"

#include "ServerMonitor.h"
#include "TeamTalkDefs.h"
#include <TeamTalkSrv.h>
#include "myace/MyACE.h"
#include "teamtalk/Commands.h"
#include "teamtalk/Common.h"
#include "teamtalk/server/Server.h"
#include "teamtalk/server/ServerNode.h"
#include "license/Trial.h"

#include <ace/ACE.h>
#include <ace/OS_Memory.h>
#include <ace/OS_NS_Thread.h>
#include <ace/OS_NS_string.h>
#include <ace/Reactor.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/SSL/SSL_Context.h>
#include <ace/Select_Reactor.h>

#include <csignal>
#include <cstddef>
#include <iostream>
#include <set>
#include <map>
#include <memory>

using teamtalk::ServerNode;

extern bool g_LicenseValid;

struct ServerInstance
{
    ACE_Select_Reactor selectReactor; 
    ACE_Reactor tcpReactor;
    ACE_Reactor udpReactor;

    std::unique_ptr<ServerMonitor> monitor;
    std::unique_ptr<ServerNode> server;

    int udp_thread, timer_thread;
    int tcp_thread{-1};

    ServerInstance(bool spawn_thread)
        : tcpReactor(&selectReactor)
         
        {
            ServerMonitor* m = nullptr;
            ServerNode* s = nullptr;
            ACE_NEW(m, ServerMonitor());
            m->m_ttInst = this;
            ACE_NEW(s, ServerNode(ACE_TEXT( TEAMTALK_VERSION ), &tcpReactor, &tcpReactor, &udpReactor, m));
            monitor.reset(m);
            server.reset(s);

            udp_thread = ACE_Thread_Manager::instance()->spawn(EventLoop, &udpReactor);
            SyncReactor(udpReactor);
            if(spawn_thread)
            {
                tcp_thread = ACE_Thread_Manager::instance()->spawn(EventLoop, &tcpReactor);
                SyncReactor(tcpReactor);
            }
        }
    ~ServerInstance()
        {
            udpReactor.end_reactor_event_loop();
            tcpReactor.end_reactor_event_loop();

            if(tcp_thread >= 0)
                ACE_Thread_Manager::instance ()->wait_grp(tcp_thread);
            ACE_Thread_Manager::instance ()->wait_grp(udp_thread);
        }
};

using servers_t = std::set<ServerInstance*>;
using thread_owners_t = std::map<TTSInstance*, ACE_thread_t>;

static servers_t servers;
static thread_owners_t threads;
static ACE_Recursive_Thread_Mutex servers_mutex;

static ServerInstance* GetServerinst(TTSInstance* pInstance)
{
    wguard_t const g(servers_mutex);

    auto const i = threads.find(pInstance);
    if(i != threads.end())
    {
        if(i->second != ACE_OS::thr_self())
        {
#if defined(UNICODE)
            std::wcerr << TEAMTALK_LIB_NAME << " server does not support multi threading. Thread " 
                       << ACE_OS::thr_self() << " != " << i->second << std::endl;
#else
            std::cerr << TEAMTALK_LIB_NAME << " server does not support multi threading. Thread " 
                      << ACE_OS::thr_self() << " != " << i->second << std::endl;
#endif
        }
    }
    threads[pInstance] = ACE_OS::thr_self();

    auto* pServer = static_cast<ServerInstance*>(pInstance);
    auto const ite = servers.find(pServer);
    if(ite != servers.end())
        return (*ite);

    return nullptr;
}

static ServerNode* GET_SERVERNODE(TTSInstance* pInstance)
{
    wguard_t const g(servers_mutex);

    ServerInstance* pServer = GetServerinst(pInstance);
    if(pServer != nullptr)
        return pServer->server.get();

    return nullptr;
}

//get ServerNode instance, lock mutex, return if not found
#define GET_SERVERNODE_RET(s, p, ret)           \
    s = GET_SERVERNODE(p);                      \
    if(!(s))return ret;                         \
    GUARD_OBJ(s, (s)->Lock())

static void InitContext()
{
#if !defined(WIN32)
    //avoid SIGPIPE
    static ACE_Sig_Action no_sigpipe ((ACE_SignalHandler) SIG_IGN);
    static ACE_Sig_Action original_action;
    no_sigpipe.register_action (SIGPIPE, &original_action);
#endif
    int const ret = ACE::set_handle_limit(-1);//client handler (must be BIG)
}

#if defined(ENABLE_ENCRYPTION)
TEAMTALKDLL_API TTBOOL TTS_SetEncryptionContext(IN TTSInstance* lpTTSInstance,
                                                IN const TTCHAR* szCertificateFile,
                                                IN const TTCHAR* szPrivateKeyFile)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, FALSE);

    EncryptionContext context = {};
    context.bVerifyClientOnce = 1;
    ACE_OS::strsncpy(context.szCertificateFile, szCertificateFile, TT_STRLEN);
    ACE_OS::strsncpy(context.szPrivateKeyFile, szPrivateKeyFile, TT_STRLEN);

    return TTS_SetEncryptionContextEx(lpTTSInstance, &context);
}

TEAMTALKDLL_API TTBOOL TTS_SetEncryptionContextEx(IN TTSInstance* lpTTSInstance,
                                                  const EncryptionContext* lpEncryptionContext)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, FALSE);

    ACE_SSL_Context* context = pServerNode->SetupEncryptionContext();
    if (context == nullptr)
        return FALSE;

    return static_cast<TTBOOL>(SetupEncryptionContext(*lpEncryptionContext, context));
}

#endif

TEAMTALKDLL_API TTSInstance* TTS_InitTeamTalk()
{
    static bool const b = false;
    if(!b)InitContext();

    LicenseCheck();

    ServerInstance* ttInst = nullptr;
    ACE_NEW_RETURN(ttInst, ServerInstance(false), nullptr);

    wguard_t const g(servers_mutex);

    servers.insert(ttInst);

    return ttInst;
}

TEAMTALKDLL_API TTBOOL TTS_CloseTeamTalk(IN TTSInstance* lpTTSInstance)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);

    wguard_t const g(servers_mutex);

    servers.erase(ttInst);
    threads.erase(ttInst);

    delete ttInst;

    return static_cast<TTBOOL>(ttInst != nullptr);
}

TEAMTALKDLL_API TTBOOL TTS_RunEventLoop(IN TTSInstance* lpTTSInstance,
                                        IN INT32* pnWaitMs)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if (!g_LicenseValid)
    {
        std::cerr << "TeamTalk SDK license has expired" << std::endl;
        return FALSE;
    }

    ACE_thread_t thrid = ACE_OS::NULL_thread;;
    if(ttInst->tcpReactor.owner(&thrid)>=0 && thrid != ACE_OS::thr_self())
        ttInst->tcpReactor.owner(ACE_OS::thr_self());

    if((pnWaitMs != nullptr) && *pnWaitMs != -1)
    {
        ACE_Time_Value tv(*pnWaitMs/1000, (*pnWaitMs % 1000) * 1000);
        return static_cast<TTBOOL>(ttInst->tcpReactor.handle_events(&tv) > 0);
    }
    
            return ttInst->tcpReactor.handle_events() > 0;
   
}

TEAMTALKDLL_API INT32 TTS_SetChannelFilesRoot(IN TTSInstance* lpTTSInstance,
                                              IN const TTCHAR* szFilesRoot,
                                              IN INT64 nMaxDiskUsage,
                                              IN INT64 nDefaultChannelQuota)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    INT32 const ret = pServerNode->SetFileSharing(szFilesRoot) ? CMDERR_SUCCESS : CMDERR_FILE_NOT_FOUND;
    if(ret == CMDERR_SUCCESS)
    {
        teamtalk::ServerSettings sprop = pServerNode->GetServerProperties();
        sprop.maxdiskusage = nMaxDiskUsage;
        sprop.diskquota = nDefaultChannelQuota;
        pServerNode->SetServerProperties(sprop);
    }
    return ret;
}

TEAMTALKDLL_API INT32 TTS_UpdateServer(IN TTSInstance* lpTTSInstance,
                                       IN const ServerProperties* lpServerProperties)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::ServerSettings sprop = pServerNode->GetServerProperties();

    Convert(*lpServerProperties, sprop);
    return pServerNode->UpdateServer(sprop).errorno;
}

TEAMTALKDLL_API INT32 TTS_MakeChannel(IN TTSInstance* lpTTSInstance,
                                      IN const Channel* lpChannel)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::ChannelProp schan;
    Convert(*lpChannel, schan);

    return pServerNode->MakeChannel(schan).errorno;
}

TEAMTALKDLL_API INT32 TTS_UpdateChannel(IN TTSInstance* lpTTSInstance,
                                        IN const Channel* lpChannel)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::ChannelProp schan;
    Convert(*lpChannel, schan);

    return pServerNode->UpdateChannel(schan).errorno;
}

TEAMTALKDLL_API INT32 TTS_RemoveChannel(IN TTSInstance* lpTTSInstance,
                                        IN INT32 nChannelID)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    return pServerNode->RemoveChannel(nChannelID).errorno;
}

TEAMTALKDLL_API INT32 TTS_AddFileToChannel(IN TTSInstance* lpTTSInstance,
                                           IN const TTCHAR* szLocalFilePath,
                                           IN const RemoteFile* lpRemoteFile)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::RemoteFile rmfile;
    Convert(*lpRemoteFile, szLocalFilePath, rmfile);

    return pServerNode->AddFileToChannel(rmfile).errorno;
}

TEAMTALKDLL_API INT32 TTS_RemoveFileFromChannel(IN TTSInstance* lpTTSInstance,
                                                IN const RemoteFile* lpRemoteFile)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    return pServerNode->RemoveFileFromChannel(lpRemoteFile->szFileName, lpRemoteFile->nChannelID).errorno;
}

TEAMTALKDLL_API INT32 TTS_MoveUser(IN TTSInstance* lpTTSInstance,
                                   IN INT32 nUserID, IN const Channel* lpChannel)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::ChannelProp prop;
    Convert(*lpChannel, prop);
    if(prop.channelid != 0)
        return pServerNode->UserJoinChannel(nUserID, prop).errorno;
            return pServerNode->UserLeaveChannel(nUserID).errorno;
}

TEAMTALKDLL_API INT32 TTS_SendTextMessage(IN TTSInstance* lpTTSInstance,
                                          const TextMessage* lpTextMessage)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::TextMessage msg;
    Convert(*lpTextMessage, msg);
    return pServerNode->SendTextMessage(msg).errorno;
}

TEAMTALKDLL_API TTBOOL TTS_StartServer(IN TTSInstance* lpTTSInstance,
                                       IN const TTCHAR* szBindIPAddr, 
                                       IN UINT16 nTcpPort, IN UINT16 nUdpPort,
                                       IN TTBOOL bEncrypted)
{
    return TTS_StartServerSysID(lpTTSInstance, szBindIPAddr, nTcpPort, nUdpPort, 
                                bEncrypted, SERVER_WELCOME);
}

TEAMTALKDLL_API TTBOOL TTS_StartServerSysID(IN TTSInstance* lpTTSInstance,
                                            IN const TTCHAR* szBindIPAddr, 
                                            IN UINT16 nTcpPort, 
                                            IN UINT16 nUdpPort,
                                            IN TTBOOL bEncrypted, 
                                            IN const TTCHAR* szSystemID)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, FALSE);

    teamtalk::ServerSettings p = pServerNode->GetServerProperties();

    p.tcpaddrs.clear();
    p.udpaddrs.clear();
    
    if((szBindIPAddr != nullptr) && (ACE_OS::strlen(szBindIPAddr) != 0u))
    {
        p.tcpaddrs.emplace_back(nTcpPort, szBindIPAddr);
        p.udpaddrs.emplace_back(nUdpPort, szBindIPAddr);
    }
    else
    {
        p.tcpaddrs.emplace_back(nTcpPort);
        p.udpaddrs.emplace_back(nUdpPort);
    }

    pServerNode->SetServerProperties(p);

    return static_cast<TTBOOL>(pServerNode->StartServer(bEncrypted != 0, szSystemID));
}

TEAMTALKDLL_API TTBOOL TTS_StopServer(IN TTSInstance* lpTTSInstance)
{
    ServerNode* pServerNode = nullptr;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, FALSE);

    pServerNode->StopServer();
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoginCallback(IN TTSInstance* lpTTSInstance,
                                                     IN UserLoginCallback* lpCallback,
                                                     IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_login_callbacks[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_login_callbacks.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserChangeNicknameCallback(IN TTSInstance* lpTTSInstance,
                                                              IN UserChangeNicknameCallback* lpCallback,
                                                              IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_changenickname_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_changenickname_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserChangeStatusCallback(IN TTSInstance* lpTTSInstance,
                                                            IN UserChangeStatusCallback* lpCallback,
                                                            IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_changestatus_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_changestatus_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserCreateUserAccountCallback(IN TTSInstance* lpTTSInstance,
                                                               IN UserCreateUserAccountCallback* lpCallback,
                                                               IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_createuseraccount_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_createuseraccount_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserDeleteUserAccountCallback(IN TTSInstance* lpTTSInstance,
                                                               IN UserDeleteUserAccountCallback* lpCallback,
                                                               IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_deleteuseraccount_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_deleteuseraccount_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserAddServerBanCallback(IN TTSInstance* lpTTSInstance,
                                                          IN UserAddServerBanCallback* lpCallback,
                                                          IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_addserverban_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_addserverban_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserAddServerBanIPAddressCallback(IN TTSInstance* lpTTSInstance,
                                                                   IN UserAddServerBanIPAddressCallback* lpCallback,
                                                                   IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_addserverbanip_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_addserverbanip_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserDeleteServerBanCallback(IN TTSInstance* lpTTSInstance,
                                                             IN UserDeleteServerBanCallback* lpCallback,
                                                             IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_deleteserverban_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_deleteserverban_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserConnectedCallback(IN TTSInstance* lpTTSInstance,
                                                       IN UserConnectedCallback* lpCallback,
                                                       IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userconnected[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userconnected.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoggedInCallback(IN TTSInstance* lpTTSInstance,
                                                      IN UserLoggedInCallback* lpCallback,
                                                      IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userloggedin[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userloggedin.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoggedOutCallback(IN TTSInstance* lpTTSInstance,
                                                       IN UserLoggedOutCallback* lpCallback,
                                                       IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userloggedout[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userloggedout.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserDisconnectedCallback(IN TTSInstance* lpTTSInstance,
                                                          IN UserDisconnectedCallback* lpCallback,
                                                          IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userdisconnected[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userdisconnected.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserTimedoutCallback(IN TTSInstance* lpTTSInstance,
                                                      IN UserTimedoutCallback* lpCallback,
                                                      IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_usertimedout[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_usertimedout.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserKickedCallback(IN TTSInstance* lpTTSInstance,
                                                    IN UserKickedCallback* lpCallback,
                                                    IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userkicked[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userkicked.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserBannedCallback(IN TTSInstance* lpTTSInstance,
                                                    IN UserBannedCallback* lpCallback,
                                                    IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userbanned[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userbanned.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserUnbannedCallback(IN TTSInstance* lpTTSInstance,
                                                      IN UserUnbannedCallback* lpCallback,
                                                      IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userunbanned[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userunbanned.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                     IN UserUpdatedCallback* lpCallback,
                                                     IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userupdate[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userupdate.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserJoinedChannelCallback(IN TTSInstance* lpTTSInstance,
                                                           IN UserJoinedChannelCallback* lpCallback,
                                                           IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userjoined[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userjoined.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserLeftChannelCallback(IN TTSInstance* lpTTSInstance,
                                                         IN UserLeftChannelCallback* lpCallback,
                                                         IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_userleft[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userleft.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserMovedCallback(IN TTSInstance* lpTTSInstance,
                                                   IN UserMovedCallback* lpCallback,
                                                   IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_usermoved[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_usermoved.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserTextMessageCallback(IN TTSInstance* lpTTSInstance,
                                                         IN UserTextMessageCallback* lpCallback,
                                                         IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_usertextmsg[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_usertextmsg.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterChannelCreatedCallback(IN TTSInstance* lpTTSInstance,
                                                        IN ChannelCreatedCallback* lpCallback,
                                                        IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_chancreated[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_chancreated.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterChannelUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                        IN ChannelUpdatedCallback* lpCallback,
                                                        IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_chanupdated[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_chanupdated.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterChannelRemovedCallback(IN TTSInstance* lpTTSInstance,
                                                        IN ChannelRemovedCallback* lpCallback,
                                                        IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_chanremoved[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_chanremoved.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterFileUploadedCallback(IN TTSInstance* lpTTSInstance,
                                                      IN FileUploadedCallback* lpCallback,
                                                      IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_fileupload[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_fileupload.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterFileDownloadedCallback(IN TTSInstance* lpTTSInstance,
                                                        IN FileDownloadedCallback* lpCallback,
                                                        IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_filedownload[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_filedownload.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterFileDeletedCallback(IN TTSInstance* lpTTSInstance,
                                                     IN FileDeletedCallback* lpCallback,
                                                     IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_filedelete[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_filedelete.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterServerUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                       IN ServerUpdatedCallback* lpCallback,
                                                       IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_serverupdated[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_serverupdated.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterSaveServerConfigCallback(IN TTSInstance* lpTTSInstance,
                                                          IN SaveServerConfigCallback* lpCallback,
                                                          IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GetServerinst(lpTTSInstance);
    if(ttInst == nullptr)
        return FALSE;

    if(bEnable != 0)
        ttInst->monitor->m_saveservercfg[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_saveservercfg.erase(lpUserData);

    return TRUE;
}
