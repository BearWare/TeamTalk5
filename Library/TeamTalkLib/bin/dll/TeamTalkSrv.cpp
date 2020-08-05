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
#include <ace/Reactor.h>
#include <ace/Select_Reactor.h>

#include "TeamTalkSrv.h"
#include "ServerMonitor.h"
#include "Convert.h"

#include <teamtalk/ttassert.h>

#include <memory>
#include <iostream>

using teamtalk::ServerNode;

struct ServerInstance
{
    ACE_Select_Reactor selectReactor; 
    ACE_Reactor tcpReactor;
    ACE_Reactor udpReactor;

    std::unique_ptr<ServerMonitor> monitor;
    std::unique_ptr<ServerNode> server;

    int udp_thread, timer_thread;
    int tcp_thread;

    ServerInstance(bool spawn_thread)
        : tcpReactor(&selectReactor)
        , tcp_thread(-1)
        {
            ServerMonitor* m;
            ServerNode* s;
            ACE_NEW(m, ServerMonitor());
            m->m_ttInst = this;
            ACE_NEW(s, ServerNode(ACE_TEXT( TEAMTALK_VERSION ), &tcpReactor, &tcpReactor, &udpReactor, m));
            monitor.reset(m);
            server.reset(s);

            udp_thread = ACE_Thread_Manager::instance()->spawn(event_loop, &udpReactor);
            SyncReactor(udpReactor);
            if(spawn_thread)
            {
                tcp_thread = ACE_Thread_Manager::instance()->spawn(event_loop, &tcpReactor);
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

typedef std::set<ServerInstance*> servers_t;
typedef std::map<TTSInstance*, ACE_thread_t> thread_owners_t;

servers_t servers;
thread_owners_t threads;
ACE_Recursive_Thread_Mutex servers_mutex;

ServerInstance* GET_SERVERINST(TTSInstance* pInstance)
{
    wguard_t g(servers_mutex);

    thread_owners_t::iterator i = threads.find(pInstance);
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

    ServerInstance* pServer = static_cast<ServerInstance*>(pInstance);
    servers_t::iterator ite = servers.find(pServer);
    if(ite != servers.end())
        return (*ite);

    return NULL;
}

ServerNode* GET_SERVERNODE(TTSInstance* pInstance)
{
    wguard_t g(servers_mutex);

    ServerInstance* pServer = GET_SERVERINST(pInstance);
    if(pServer)
        return pServer->server.get();

    return NULL;
}

//get ServerNode instance, lock mutex, return if not found
#define GET_SERVERNODE_RET(s, p, ret)           \
    s = GET_SERVERNODE(p);                      \
    if(!s)return ret;                           \
    GUARD_OBJ(s, s->lock())

void InitContext()
{
#if !defined(WIN32)
    //avoid SIGPIPE
    static ACE_Sig_Action no_sigpipe ((ACE_SignalHandler) SIG_IGN);
    static ACE_Sig_Action original_action;
    no_sigpipe.register_action (SIGPIPE, &original_action);
#endif
    int ret = ACE::set_handle_limit(-1);//client handler (must be BIG)
}

#if defined(ENABLE_ENCRYPTION)
TEAMTALKDLL_API TTBOOL TTS_SetEncryptionContext(IN TTSInstance* lpTTSInstance,
                                                IN const TTCHAR* szCertificateFile,
                                                IN const TTCHAR* szPrivateKeyFile)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, FALSE);

    if (szCertificateFile && szPrivateKeyFile)
    {
        ACE_SSL_Context* ssl_context = pServerNode->SetupEncryptionContext();
        if (!ssl_context)
            return FALSE;

        if (ssl_context->set_mode(ACE_SSL_Context::SSLv23) < 0)
            return FALSE;

#if defined(UNICODE)
        ACE_CString cert = UnicodeToLocal(szCertificateFile);
        ACE_CString priv = UnicodeToLocal(szPrivateKeyFile);
#else
        ACE_CString cert = szCertificateFile;
        ACE_CString priv = szPrivateKeyFile;
#endif
        if (ssl_context->certificate(cert.c_str(), SSL_FILETYPE_PEM) < 0)
            return FALSE;
        if (ssl_context->private_key(priv.c_str(), SSL_FILETYPE_PEM) < 0)
            return FALSE;

        ssl_context->set_verify_peer(false, true, 0);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

TEAMTALKDLL_API TTBOOL TTS_SetEncryptionContextEx(IN TTSInstance* lpTTSInstance,
                                                  const EncryptionContext* lpEncryptionContext)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, FALSE);

    ACE_SSL_Context* context = pServerNode->SetupEncryptionContext();
    if (!context)
        return FALSE;

#if defined(UNICODE)
    ACE_CString cert = UnicodeToLocal(lpEncryptionContext->szCertificateFile);
    ACE_CString priv = UnicodeToLocal(lpEncryptionContext->szPrivateKeyFile);
    ACE_CString cafile = UnicodeToLocal(lpEncryptionContext->szCAFile);
    ACE_CString cadir = UnicodeToLocal(lpEncryptionContext->szCADir);
#else
    ACE_CString cert = lpEncryptionContext->szCertificateFile;
    ACE_CString priv = lpEncryptionContext->szPrivateKeyFile;
    ACE_CString cafile = lpEncryptionContext->szCAFile;
    ACE_CString cadir = lpEncryptionContext->szCADir;
#endif
    
    if (cert.length() && context->certificate(cert.c_str(), SSL_FILETYPE_PEM) < 0)
        return FALSE;

    if (priv.length() && context->private_key(priv.c_str(), SSL_FILETYPE_PEM) < 0)
        return FALSE;

    if (cafile.length() || cadir.length())
    {
        if (context->load_trusted_ca(cafile.length() ? cafile.c_str() : nullptr,
                                     cadir.length() ? cadir.c_str() : nullptr, false) < 0)
            return FALSE;
    }

    context->set_verify_peer(lpEncryptionContext->bVerifyPeer,
                             lpEncryptionContext->bVerifyClientOnce,
                             lpEncryptionContext->nVerifyDepth);
    
    return TRUE;
    
}

#endif

TEAMTALKDLL_API TTSInstance* TTS_InitTeamTalk()
{
    static bool b = false;
    if(!b)InitContext();

    ServerInstance* ttInst;
    ACE_NEW_RETURN(ttInst, ServerInstance(false), NULL);

    wguard_t g(servers_mutex);

    servers.insert(ttInst);

    return ttInst;
}

TEAMTALKDLL_API TTBOOL TTS_CloseTeamTalk(IN TTSInstance* lpTTSInstance)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);

    wguard_t g(servers_mutex);

    servers.erase(ttInst);
    threads.erase(ttInst);

    delete ttInst;

    return ttInst != NULL;
}

TEAMTALKDLL_API TTBOOL TTS_RunEventLoop(IN TTSInstance* lpTTSInstance,
                                        IN INT32* pnWaitMs)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    ACE_thread_t thrid = ACE_OS::NULL_thread;;
    if(ttInst->tcpReactor.owner(&thrid)>=0 && thrid != ACE_OS::thr_self())
        ttInst->tcpReactor.owner(ACE_OS::thr_self());

    if(pnWaitMs && *pnWaitMs != -1)
    {
        ACE_Time_Value tv(*pnWaitMs/1000, (*pnWaitMs % 1000) * 1000);
        return ttInst->tcpReactor.handle_events(&tv) > 0;
    }
    else
    {
        return ttInst->tcpReactor.handle_events() > 0;
    }
}

TEAMTALKDLL_API INT32 TTS_SetChannelFilesRoot(IN TTSInstance* lpTTSInstance,
                                              IN const TTCHAR* szFilesRoot,
                                              IN INT64 nMaxDiskUsage,
                                              IN INT64 nDefaultChannelQuota)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    INT32 ret = pServerNode->SetFileSharing(szFilesRoot) ? CMDERR_SUCCESS : CMDERR_FILE_NOT_FOUND;
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
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::ServerSettings sprop = pServerNode->GetServerProperties();

    Convert(*lpServerProperties, sprop);
    return pServerNode->UpdateServer(sprop).errorno;
}

TEAMTALKDLL_API INT32 TTS_MakeChannel(IN TTSInstance* lpTTSInstance,
                                      IN const Channel* lpChannel)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::ChannelProp schan;
    Convert(*lpChannel, schan);

    return pServerNode->MakeChannel(schan).errorno;
}

TEAMTALKDLL_API INT32 TTS_UpdateChannel(IN TTSInstance* lpTTSInstance,
                                        IN const Channel* lpChannel)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::ChannelProp schan;
    Convert(*lpChannel, schan);

    return pServerNode->UpdateChannel(schan).errorno;
}

TEAMTALKDLL_API INT32 TTS_RemoveChannel(IN TTSInstance* lpTTSInstance,
                                        IN INT32 nChannelID)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    return pServerNode->RemoveChannel(nChannelID).errorno;
}

TEAMTALKDLL_API INT32 TTS_AddFileToChannel(IN TTSInstance* lpTTSInstance,
                                           IN const TTCHAR* szLocalFilePath,
                                           IN const RemoteFile* lpRemoteFile)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::RemoteFile rmfile;
    Convert(*lpRemoteFile, szLocalFilePath, rmfile);

    return pServerNode->AddFileToChannel(rmfile).errorno;
}

TEAMTALKDLL_API INT32 TTS_RemoveFileFromChannel(IN TTSInstance* lpTTSInstance,
                                                IN const RemoteFile* lpRemoteFile)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    return pServerNode->RemoveFileFromChannel(lpRemoteFile->szFileName, lpRemoteFile->nChannelID).errorno;
}

TEAMTALKDLL_API INT32 TTS_MoveUser(IN TTSInstance* lpTTSInstance,
                                   IN INT32 nUserID, IN const Channel* lpChannel)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, -1);

    teamtalk::ChannelProp prop;
    Convert(*lpChannel, prop);
    if(prop.channelid != 0)
        return pServerNode->UserJoinChannel(nUserID, prop).errorno;
    else
        return pServerNode->UserLeaveChannel(nUserID).errorno;
}

TEAMTALKDLL_API INT32 TTS_SendTextMessage(IN TTSInstance* lpTTSInstance,
                                          const TextMessage* lpTextMessage)
{
    ServerNode* pServerNode;
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
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, FALSE);

    teamtalk::ServerSettings p = pServerNode->GetServerProperties();

    p.tcpaddrs.clear();
    p.udpaddrs.clear();
    
    if(szBindIPAddr && ACE_OS::strlen(szBindIPAddr))
    {
        p.tcpaddrs.push_back(ACE_INET_Addr(nTcpPort, szBindIPAddr));
        p.udpaddrs.push_back(ACE_INET_Addr(nUdpPort, szBindIPAddr));
    }
    else
    {
        p.tcpaddrs.push_back(ACE_INET_Addr(nTcpPort));
        p.udpaddrs.push_back(ACE_INET_Addr(nUdpPort));
    }

    pServerNode->SetServerProperties(p);

    return pServerNode->StartServer(bEncrypted, szSystemID);
}

TEAMTALKDLL_API TTBOOL TTS_StopServer(IN TTSInstance* lpTTSInstance)
{
    ServerNode* pServerNode;
    GET_SERVERNODE_RET(pServerNode, lpTTSInstance, FALSE);

    pServerNode->StopServer();
    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoginCallback(IN TTSInstance* lpTTSInstance,
                                                     IN UserLoginCallback* lpCallback,
                                                     IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_login_callbacks[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_login_callbacks.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserChangeNicknameCallback(IN TTSInstance* lpTTSInstance,
                                                              IN UserChangeNicknameCallback* lpCallback,
                                                              IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_changenickname_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_changenickname_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserChangeStatusCallback(IN TTSInstance* lpTTSInstance,
                                                            IN UserChangeStatusCallback* lpCallback,
                                                            IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_changestatus_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_changestatus_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserCreateUserAccountCallback(IN TTSInstance* lpTTSInstance,
                                                               IN UserCreateUserAccountCallback* lpCallback,
                                                               IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_createuseraccount_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_createuseraccount_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserDeleteUserAccountCallback(IN TTSInstance* lpTTSInstance,
                                                               IN UserDeleteUserAccountCallback* lpCallback,
                                                               IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_deleteuseraccount_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_deleteuseraccount_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserAddServerBanCallback(IN TTSInstance* lpTTSInstance,
                                                          IN UserAddServerBanCallback* lpCallback,
                                                          IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_addserverban_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_addserverban_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserAddServerBanIPAddressCallback(IN TTSInstance* lpTTSInstance,
                                                                   IN UserAddServerBanIPAddressCallback* lpCallback,
                                                                   IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_addserverbanip_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_addserverbanip_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserDeleteServerBanCallback(IN TTSInstance* lpTTSInstance,
                                                             IN UserDeleteServerBanCallback* lpCallback,
                                                             IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_deleteserverban_callback[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_deleteserverban_callback.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserConnectedCallback(IN TTSInstance* lpTTSInstance,
                                                       IN UserConnectedCallback* lpCallback,
                                                       IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userconnected[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userconnected.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoggedInCallback(IN TTSInstance* lpTTSInstance,
                                                      IN UserLoggedInCallback* lpCallback,
                                                      IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userloggedin[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userloggedin.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserLoggedOutCallback(IN TTSInstance* lpTTSInstance,
                                                       IN UserLoggedOutCallback* lpCallback,
                                                       IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userloggedout[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userloggedout.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserDisconnectedCallback(IN TTSInstance* lpTTSInstance,
                                                          IN UserDisconnectedCallback* lpCallback,
                                                          IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userdisconnected[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userdisconnected.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserTimedoutCallback(IN TTSInstance* lpTTSInstance,
                                                      IN UserTimedoutCallback* lpCallback,
                                                      IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_usertimedout[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_usertimedout.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserKickedCallback(IN TTSInstance* lpTTSInstance,
                                                    IN UserKickedCallback* lpCallback,
                                                    IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userkicked[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userkicked.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserBannedCallback(IN TTSInstance* lpTTSInstance,
                                                    IN UserBannedCallback* lpCallback,
                                                    IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userbanned[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userbanned.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserUnbannedCallback(IN TTSInstance* lpTTSInstance,
                                                      IN UserUnbannedCallback* lpCallback,
                                                      IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userunbanned[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userunbanned.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                     IN UserUpdatedCallback* lpCallback,
                                                     IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userupdate[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userupdate.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserJoinedChannelCallback(IN TTSInstance* lpTTSInstance,
                                                           IN UserJoinedChannelCallback* lpCallback,
                                                           IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userjoined[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userjoined.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserLeftChannelCallback(IN TTSInstance* lpTTSInstance,
                                                         IN UserLeftChannelCallback* lpCallback,
                                                         IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_userleft[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_userleft.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserMovedCallback(IN TTSInstance* lpTTSInstance,
                                                   IN UserMovedCallback* lpCallback,
                                                   IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_usermoved[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_usermoved.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterUserTextMessageCallback(IN TTSInstance* lpTTSInstance,
                                                         IN UserTextMessageCallback* lpCallback,
                                                         IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_usertextmsg[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_usertextmsg.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterChannelCreatedCallback(IN TTSInstance* lpTTSInstance,
                                                        IN ChannelCreatedCallback* lpCallback,
                                                        IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_chancreated[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_chancreated.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterChannelUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                        IN ChannelUpdatedCallback* lpCallback,
                                                        IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_chanupdated[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_chanupdated.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterChannelRemovedCallback(IN TTSInstance* lpTTSInstance,
                                                        IN ChannelRemovedCallback* lpCallback,
                                                        IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_chanremoved[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_chanremoved.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterFileUploadedCallback(IN TTSInstance* lpTTSInstance,
                                                      IN FileUploadedCallback* lpCallback,
                                                      IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_fileupload[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_fileupload.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterFileDownloadedCallback(IN TTSInstance* lpTTSInstance,
                                                        IN FileDownloadedCallback* lpCallback,
                                                        IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_filedownload[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_filedownload.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterFileDeletedCallback(IN TTSInstance* lpTTSInstance,
                                                     IN FileDeletedCallback* lpCallback,
                                                     IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_filedelete[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_filedelete.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterServerUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                                       IN ServerUpdatedCallback* lpCallback,
                                                       IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_serverupdated[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_serverupdated.erase(lpUserData);

    return TRUE;
}

TEAMTALKDLL_API TTBOOL TTS_RegisterSaveServerConfigCallback(IN TTSInstance* lpTTSInstance,
                                                          IN SaveServerConfigCallback* lpCallback,
                                                          IN VOID* lpUserData, IN TTBOOL bEnable)
{
    ServerInstance* ttInst = GET_SERVERINST(lpTTSInstance);
    if(!ttInst)
        return FALSE;

    if(bEnable)
        ttInst->monitor->m_saveservercfg[lpUserData] = lpCallback;
    else
        ttInst->monitor->m_saveservercfg.erase(lpUserData);

    return TRUE;
}
