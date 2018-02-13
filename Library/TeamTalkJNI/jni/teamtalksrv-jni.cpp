/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "ttconvert-jni.h"

#include <string.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <map>

#include <TeamTalkSrv.h>

using namespace std;


typedef map<TTSInstance*, JNIEnv*> jenv_t;
typedef map<TTSInstance*, jobject> refs_t;
jenv_t envs;
refs_t command_refs, logger_refs;

extern "C" {
    
    /* Client command callbacks */

    void userLoginCallback(IN TTSInstance* lpTTSInstance, 
                           IN VOID* lpUserData,
                           OUT ClientErrorMsg* lpClientErrorMsg,
                           IN const User* lpUser, 
                           IN OUT UserAccount* lpUserAccount) {
        
        JNIEnv* env = envs[lpTTSInstance];

        jobject errmsg_obj = newClientErrorMsg(env, lpClientErrorMsg);
        assert(errmsg_obj);
        
        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jobject useraccount_obj = newUserAccount(env, lpUserAccount);
        assert(useraccount_obj);
        
        jclass cls = env->FindClass("dk/bearware/ServerCallback");
        jmethodID method = env->GetMethodID(cls, "userLogin", 
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;Ldk/bearware/UserAccount;)V");
        assert(method);

        jobject cb_obj = command_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, errmsg_obj, user_obj, useraccount_obj);
        assert(com_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
        setUserAccount(env, *lpUserAccount, useraccount_obj, J2N);
    }
    void userChangeNicknameCallback(IN TTSInstance* lpTTSInstance, 
                                    IN VOID* lpUserData,
                                    OUT ClientErrorMsg* lpClientErrorMsg,
                                    IN const User* lpUser, 
                                    IN const TTCHAR* szNewNickname)
    {
        JNIEnv* env = envs[lpTTSInstance];

        jobject errmsg_obj = newClientErrorMsg(env, lpClientErrorMsg);
        assert(errmsg_obj);
        
        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jclass cls = env->FindClass("dk/bearware/ServerCallback");
        jmethodID method = env->GetMethodID(cls, "userChangeNickname",
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;Ljava/lang/String;)V");
        assert(method);

        jstring newnick_obj = NEW_JSTRING(env, szNewNickname);

        jobject cb_obj = command_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, errmsg_obj, user_obj, newnick_obj);
        assert(com_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userChangeStatusCallback(IN TTSInstance* lpTTSInstance, 
                                  IN VOID* lpUserData,
                                  OUT ClientErrorMsg* lpClientErrorMsg,
                                  IN const User* lpUser, 
                                  IN int nNewStatusMode,
                                  IN const TTCHAR* szNewStatusMsg)
    {
        JNIEnv* env = envs[lpTTSInstance];

        jobject errmsg_obj = newClientErrorMsg(env, lpClientErrorMsg);
        assert(errmsg_obj);
        
        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jclass cls = env->FindClass("dk/bearware/ServerCallback");
        jmethodID method = env->GetMethodID(cls, "userChangeStatus", 
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;ILjava/lang/String;)V");
        assert(method);

        jstring newstatusmsg_obj = NEW_JSTRING(env, szNewStatusMsg);

        jobject cb_obj = command_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, errmsg_obj, user_obj, nNewStatusMode, newstatusmsg_obj);
        assert(com_obj);
        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userCreateUserAccountCallback(IN TTSInstance* lpTTSInstance, 
                                       IN VOID* lpUserData,
                                       OUT ClientErrorMsg* lpClientErrorMsg,
                                       IN const User* lpUser, 
                                       IN const UserAccount* lpUserAccount) {

        JNIEnv* env = envs[lpTTSInstance];

        jobject errmsg_obj = newClientErrorMsg(env, lpClientErrorMsg);
        assert(errmsg_obj);
        
        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jobject useraccount_obj = newUserAccount(env, lpUserAccount);
        assert(useraccount_obj);

        jclass cls = env->FindClass("dk/bearware/ServerCallback");
        jmethodID method = env->GetMethodID(cls, "userCreateUserAccount", 
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;Ldk/bearware/UserAccount;)V");
        assert(method);

        jobject cb_obj = command_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, errmsg_obj, user_obj, useraccount_obj);
        assert(com_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userDeleteUserAccountCallback(IN TTSInstance* lpTTSInstance, 
                                       IN VOID* lpUserData,
                                       OUT ClientErrorMsg* lpClientErrorMsg,
                                       IN const User* lpUser, 
                                       IN const TTCHAR* szUsername) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject errmsg_obj = newClientErrorMsg(env, lpClientErrorMsg);
        assert(errmsg_obj);
        
        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jstring username_obj = NEW_JSTRING(env, szUsername);

        jclass cls = env->FindClass("dk/bearware/ServerCallback");
        jmethodID method = env->GetMethodID(cls, "userDeleteUserAccount", 
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;Ljava/lang/String;)V");
        assert(method);

        jobject cb_obj = command_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, errmsg_obj, user_obj, username_obj);
        assert(com_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userAddServerBanCallback(IN TTSInstance* lpTTSInstance, 
                                  IN VOID* lpUserData,
                                  OUT ClientErrorMsg* lpClientErrorMsg,
                                  IN const User* lpBanner, 
                                  IN const User* lpBanee) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject errmsg_obj = newClientErrorMsg(env, lpClientErrorMsg);
        assert(errmsg_obj);
        
        jobject banner_obj = newUser(env, lpBanner);
        jobject banee_obj = newUser(env, lpBanee);
        assert(banner_obj);
        assert(banee_obj);

        jclass cls = env->FindClass("dk/bearware/ServerCallback");
        jmethodID method = env->GetMethodID(cls, "userAddServerBan", 
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = command_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, errmsg_obj, banner_obj, banee_obj);
        assert(com_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userAddServerBanIPAddressCallback(IN TTSInstance* lpTTSInstance, 
                                           IN VOID* lpUserData,
                                           OUT ClientErrorMsg* lpClientErrorMsg,
                                           IN const User* lpBanner, 
                                           IN const TTCHAR* szIPAddress) {

        JNIEnv* env = envs[lpTTSInstance];

        jobject errmsg_obj = newClientErrorMsg(env, lpClientErrorMsg);
        assert(errmsg_obj);
        
        jobject banner_obj = newUser(env, lpBanner);
        assert(lpBanner);
        jstring ipaddr_str = NEW_JSTRING(env, szIPAddress);

        jclass cls = env->FindClass("dk/bearware/ServerCallback");
        jmethodID method = env->GetMethodID(cls, "userAddServerBanIPAddress", 
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;Ljava/lang/String;)V");
        assert(method);

        jobject cb_obj = command_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, errmsg_obj, banner_obj, ipaddr_str);
        assert(com_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userDeleteServerBanCallback(IN TTSInstance* lpTTSInstance, 
                                     IN VOID* lpUserData,
                                     OUT ClientErrorMsg* lpClientErrorMsg,
                                     IN const User* lpUser, 
                                     IN const TTCHAR* szIPAddress) {

        assert(lpUser);
        assert(szIPAddress);

        JNIEnv* env = envs[lpTTSInstance];

        jobject errmsg_obj = newClientErrorMsg(env, lpClientErrorMsg);
        assert(errmsg_obj);
        
        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jstring ipaddr_str = NEW_JSTRING(env, szIPAddress);

        jclass cls = env->FindClass("dk/bearware/ServerCallback");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userDeleteServerBan", 
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;Ljava/lang/String;)V");
        assert(method);

        jobject cb_obj = command_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, errmsg_obj, user_obj, ipaddr_str);
        assert(com_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }

    /* Log events */

    void logUserConnectedCallback(IN TTSInstance* lpTTSInstance,
                                  IN VOID* lpUserData, IN const User* lpUser) {

        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);
        
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userConnected", 
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj);
        assert(com_obj);
    }
    void logUserLoggedInCallback(IN TTSInstance* lpTTSInstance,
                                 IN VOID* lpUserData, IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userLoggedIn", 
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj);
        assert(com_obj);
    }
    void logUserLoggedOutCallback(IN TTSInstance* lpTTSInstance,
                                  IN VOID* lpUserData, IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userLoggedOut", 
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj);
        assert(com_obj);
    }
    void logUserDisconnectedCallback(IN TTSInstance* lpTTSInstance,
                                     IN VOID* lpUserData, IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userDisconnected", 
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj);
        assert(com_obj);
    }
    void logUserTimedoutCallback(IN TTSInstance* lpTTSInstance,
                                 IN VOID* lpUserData, IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userTimedout", 
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj);
        assert(com_obj);
    }
    void logUserKickedCallback(IN TTSInstance* lpTTSInstance,
                               IN VOID* lpUserData, IN const User* lpKicker,
                               IN const User* lpKickee, IN const Channel* lpChannel) {

        JNIEnv* env = envs[lpTTSInstance];

        jobject kicker_obj = lpKicker? newUser(env, lpKicker) : 0;
        assert(lpKickee);
        jobject kickee_obj = newUser(env, lpKickee);
        jobject channel_obj = lpChannel? newChannel(env, lpChannel) : 0;
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userKicked", 
                                            "(Ldk/bearware/User;Ldk/bearware/User;Ldk/bearware/Channel;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, kicker_obj, kickee_obj, channel_obj);
        assert(com_obj);
    }
    void logUserBannedCallback(IN TTSInstance* lpTTSInstance,
                               IN VOID* lpUserData, IN const User* lpBanner,
                               IN const User* lpBanee, IN const Channel* lpChannel) {

        JNIEnv* env = envs[lpTTSInstance];

        jobject banner_obj = newUser(env, lpBanner);
        jobject banee_obj = newUser(env, lpBanee);
        jobject channel_obj = newChannel(env, lpChannel);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userBanned", 
                                            "(Ldk/bearware/User;Ldk/bearware/User;Ldk/bearware/Channel;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, banner_obj, banee_obj, channel_obj);
        assert(com_obj);
    }
    void logUserUnbannedCallback(IN TTSInstance* lpTTSInstance,
                                 IN VOID* lpUserData, IN const User* lpUnbanner,
                                 IN const TTCHAR* szIPAddress) {

        JNIEnv* env = envs[lpTTSInstance];
        jclass cls = env->FindClass("dk/bearware/User");
        assert(cls);

        jobject banner_obj = newUser(env, lpUnbanner);

        jstring ipaddr_str = NEW_JSTRING(env, szIPAddress);

        cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userUnbanned", 
                                            "(Ldk/bearware/User;Ljava/lang/String;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, banner_obj, ipaddr_str);
        assert(com_obj);
    }
    void logUserUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                IN VOID* lpUserData, IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);
        
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userUpdated", 
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj);
        assert(com_obj);
    }
    void logUserJoinedChannelCallback(IN TTSInstance* lpTTSInstance,
                                      IN VOID* lpUserData, IN const User* lpUser,
                                      IN const Channel* lpChannel) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);
        jobject chan_obj = newChannel(env, lpChannel);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userJoinedChannel", 
                                            "(Ldk/bearware/User;Ldk/bearware/Channel;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj, chan_obj);
        assert(com_obj);
    }
    void logUserLeftChannelCallback(IN TTSInstance* lpTTSInstance,
                                    IN VOID* lpUserData, IN const User* lpUser,
                                    IN const Channel* lpChannel) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);
        jobject chan_obj = newChannel(env, lpChannel);
        assert(chan_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userLeftChannel", 
                                            "(Ldk/bearware/User;Ldk/bearware/Channel;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj, chan_obj);
        assert(com_obj);
    }
    void logUserMovedCallback(IN TTSInstance* lpTTSInstance,
                              IN VOID* lpUserData, IN const User* lpMover,
                              IN const User* lpMovee) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject mover_obj = newUser(env, lpMover);
        assert(mover_obj);
        jobject movee_obj = newUser(env, lpMovee);
        assert(movee_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userMoved", 
                                            "(Ldk/bearware/User;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, mover_obj, movee_obj);
        assert(com_obj);
    }
    void logUserTextMessageCallback(IN TTSInstance* lpTTSInstance,
                                    IN VOID* lpUserData, IN const User* lpUser,
                                    IN const TextMessage* lpTextMessage) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);
        jobject tm_obj = newTextMessage(env, lpTextMessage);
        assert(tm_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userTextMessage", 
                                            "(Ldk/bearware/User;Ldk/bearware/TextMessage;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj, tm_obj);
        assert(com_obj);
    }
    void logChannelCreatedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* lpUserData, IN const Channel* lpChannel,
                                   IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = lpUser ? newUser(env, lpUser) : 0;
        assert(lpChannel);
        jobject chan_obj = newChannel(env, lpChannel);
        assert(chan_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "channelCreated", 
                                            "(Ldk/bearware/Channel;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, chan_obj, user_obj);
        assert(com_obj);
    }
    void logChannelUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* lpUserData, IN const Channel* lpChannel,
                                   IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = lpUser ? newUser(env, lpUser) : 0;
        assert(lpChannel);
        jobject chan_obj = newChannel(env, lpChannel);
        assert(chan_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "channelUpdated", 
                                            "(Ldk/bearware/Channel;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, chan_obj, user_obj);
        assert(com_obj);
    }
    void logChannelRemovedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* lpUserData, IN const Channel* lpChannel,
                                   IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = lpUser ? newUser(env, lpUser) : 0;
        assert(lpChannel);
        jobject chan_obj = newChannel(env, lpChannel);
        assert(chan_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "channelRemoved", 
                                            "(Ldk/bearware/Channel;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, chan_obj, user_obj);
        assert(com_obj);
    }
    void logFileUploadedCallback(IN TTSInstance* lpTTSInstance,
                                 IN VOID* lpUserData, 
                                 IN const RemoteFile* lpRemoteFile,
                                 IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);
        jobject file_obj = newRemoteFile(env, lpRemoteFile);
        assert(file_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "fileUploaded", 
                                            "(Ldk/bearware/RemoteFile;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, file_obj, user_obj);
        assert(com_obj);
    }
    void logFileDownloadedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* lpUserData, 
                                   IN const RemoteFile* lpRemoteFile,
                                   IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);
        jobject file_obj = newRemoteFile(env, lpRemoteFile);
        assert(file_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "fileDownloaded", 
                                            "(Ldk/bearware/RemoteFile;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, file_obj, user_obj);
        assert(com_obj);
    }
    void logFileDeletedCallback(IN TTSInstance* lpTTSInstance,
                                IN VOID* lpUserData, 
                                IN const RemoteFile* lpRemoteFile,
                                IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);
        jobject file_obj = newRemoteFile(env, lpRemoteFile);
        assert(file_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "fileDeleted", 
                                            "(Ldk/bearware/RemoteFile;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, file_obj, user_obj);
        assert(com_obj);
    }
    void logServerUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                  IN VOID* lpUserData, 
                                  IN const ServerProperties* lpServerProperties,
                                  IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jobject sp_obj = newServerProperties(env, lpServerProperties);
        assert(sp_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "serverUpdated", 
                                            "(Ldk/bearware/ServerProperties;Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, sp_obj, user_obj);
        assert(com_obj);
    }
    void logSaveServerConfigCallback(IN TTSInstance* lpTTSInstance,
                                     IN VOID* lpUserData, 
                                     IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        assert(user_obj);

        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "serverSavedConfig", 
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        jobject com_obj = env->CallObjectMethod(cb_obj, method, user_obj);
        assert(com_obj);
    }

    JNIEXPORT jstring JNICALL Java_dk_bearware_TeamTalkSrv_getVersion(JNIEnv* env,
                                                                      jclass)
    {
        const TTCHAR* ttv = TT_GetVersion();
        return NEW_JSTRING(env, ttv);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_setLicenseInformation(JNIEnv* env,
                                                                                   jclass,
                                                                                   jstring szRegName,
                                                                                   jstring szRegKey)
    {
        THROW_NULLEX(env, szRegName, false);
        THROW_NULLEX(env, szRegKey, false);
        return TT_SetLicenseInformation(ttstr(env, szRegName), ttstr(env, szRegKey));
    }

    JNIEXPORT jlong JNICALL Java_dk_bearware_TeamTalkSrv_initTeamTalk
    (JNIEnv *env, jobject thiz) {

        TTSInstance* ttsInst = TTS_InitTeamTalk();

        envs[ttsInst] = env;

        return jlong(ttsInst);
    }

    JNIEXPORT void JNICALL Java_dk_bearware_TeamTalkSrv_closeTeamTalk
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance) {
        TTSInstance* inst = reinterpret_cast<TTSInstance*>(lpTTSInstance);

        TTS_CloseTeamTalk(inst);
        
        if(command_refs[inst])
            env->DeleteGlobalRef(command_refs[inst]);
        if(logger_refs[inst])
            env->DeleteGlobalRef(logger_refs[inst]);

        command_refs.erase(inst);
        logger_refs.erase(inst);
        envs.erase(inst);
    }

    JNIEXPORT void JNICALL Java_dk_bearware_TeamTalkSrv_registerServerCallback
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jobject servercallback) {

        TTSInstance* inst = reinterpret_cast<TTSInstance*>(lpTTSInstance);

        command_refs[inst] = env->NewGlobalRef(servercallback);

        TTS_RegisterUserLoginCallback(inst, userLoginCallback,
                                      servercallback, true);
        TTS_RegisterUserChangeNicknameCallback(inst, userChangeNicknameCallback,
                                               servercallback, true);
        TTS_RegisterUserChangeStatusCallback(inst, userChangeStatusCallback,
                                             servercallback, true);
        TTS_RegisterUserCreateUserAccountCallback(inst, userCreateUserAccountCallback,
                                                  servercallback, true);
        TTS_RegisterUserDeleteUserAccountCallback(inst, userDeleteUserAccountCallback,
                                                  servercallback, true);
        TTS_RegisterUserAddServerBanCallback(inst, userAddServerBanCallback,
                                             servercallback, true);
        TTS_RegisterUserAddServerBanIPAddressCallback(inst, userAddServerBanIPAddressCallback,
                                                      servercallback, true);
        TTS_RegisterUserDeleteServerBanCallback(inst, userDeleteServerBanCallback,
                                                servercallback, true);
    }

    JNIEXPORT void JNICALL Java_dk_bearware_TeamTalkSrv_registerServerLogger
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jobject serverlogger) {

        TTSInstance* inst = reinterpret_cast<TTSInstance*>(lpTTSInstance);

        logger_refs[inst] = env->NewGlobalRef(serverlogger);
        
        TTS_RegisterUserConnectedCallback(inst, logUserConnectedCallback, 0, true);
        TTS_RegisterUserLoggedInCallback(inst, logUserLoggedInCallback, 0, true);
        TTS_RegisterUserLoggedOutCallback(inst, logUserLoggedOutCallback, 0, true);
        TTS_RegisterUserDisconnectedCallback(inst, logUserDisconnectedCallback, 0, true);
        TTS_RegisterUserTimedoutCallback(inst, logUserTimedoutCallback, 0, true);
        TTS_RegisterUserKickedCallback(inst, logUserKickedCallback, 0, true);
        TTS_RegisterUserBannedCallback(inst, logUserBannedCallback, 0, true);
        TTS_RegisterUserUnbannedCallback(inst, logUserUnbannedCallback, 0, true);
        TTS_RegisterUserUpdatedCallback(inst, logUserUpdatedCallback, 0, true);
        TTS_RegisterUserJoinedChannelCallback(inst, logUserJoinedChannelCallback, 0, true);
        TTS_RegisterUserLeftChannelCallback(inst, logUserLeftChannelCallback, 0, true);
        TTS_RegisterUserMovedCallback(inst, logUserMovedCallback, 0, true);
        TTS_RegisterUserTextMessageCallback(inst, logUserTextMessageCallback, 0, true);
        TTS_RegisterChannelCreatedCallback(inst, logChannelCreatedCallback, 0, true);
        TTS_RegisterChannelUpdatedCallback(inst, logChannelUpdatedCallback, 0, true);
        TTS_RegisterChannelRemovedCallback(inst, logChannelRemovedCallback, 0, true);
        TTS_RegisterFileUploadedCallback(inst, logFileUploadedCallback, 0, true);
        TTS_RegisterFileDownloadedCallback(inst, logFileDownloadedCallback, 0, true);
        TTS_RegisterFileDeletedCallback(inst, logFileDeletedCallback, 0, true);
        TTS_RegisterServerUpdatedCallback(inst, logServerUpdatedCallback, 0, true);
        TTS_RegisterSaveServerConfigCallback(inst, logSaveServerConfigCallback, 0, true);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_setEncryptionContext
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jstring szCertificateFile, jstring szPrivateKeyFile) {
        return TTS_SetEncryptionContext(reinterpret_cast<TTSInstance*>(lpTTSInstance), ttstr(env, szCertificateFile),
                                        ttstr(env, szPrivateKeyFile));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_runEventLoop
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jint pnWaitMs) {

        envs[reinterpret_cast<TTSInstance*>(lpTTSInstance)] = env;

        INT32 _pnWaitMs = pnWaitMs;

        return TTS_RunEventLoop(reinterpret_cast<TTSInstance*>(lpTTSInstance), &_pnWaitMs);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_setChannelFilesRoot
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jstring szFilesRoot,
     jlong nMaxDiskUsage, jlong nDefaultChannelQuota) {

        return TTS_SetChannelFilesRoot(reinterpret_cast<TTSInstance*>(lpTTSInstance),
                                       ttstr(env, szFilesRoot), nMaxDiskUsage, nDefaultChannelQuota);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_updateServer
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jobject lpServerProperties) {

        ServerProperties srvprop;
        setServerProperties(env, srvprop, lpServerProperties, J2N);
        return TTS_UpdateServer(reinterpret_cast<TTSInstance*>(lpTTSInstance), &srvprop);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_makeChannel
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        
        return TTS_MakeChannel(reinterpret_cast<TTSInstance*>(lpTTSInstance), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_updateChannel
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        
        return TTS_UpdateChannel(reinterpret_cast<TTSInstance*>(lpTTSInstance), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_removeChannel
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jint nChannelID) {
        return TTS_RemoveChannel(reinterpret_cast<TTSInstance*>(lpTTSInstance), nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_addFileToChannel
        (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jstring szLocalFilePath, jobject lpRemoteFile) {
        RemoteFile rmfile;
        setRemoteFile(env, rmfile, lpRemoteFile, J2N);

        return TTS_AddFileToChannel(reinterpret_cast<TTSInstance*>(lpTTSInstance), 
                                    ttstr(env, szLocalFilePath), &rmfile);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_removeFileFromChannel
        (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jobject lpRemoteFile) {

        RemoteFile rmfile;
        setRemoteFile(env, rmfile, lpRemoteFile, J2N);

        return TTS_RemoveFileFromChannel(reinterpret_cast<TTSInstance*>(lpTTSInstance), 
                                         &rmfile);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_sendTextMessage
        (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jobject lpTextMessage) {

        TextMessage msg;
        setTextMessage(env, msg, lpTextMessage, J2N);

        return TTS_SendTextMessage(reinterpret_cast<TTSInstance*>(lpTTSInstance), 
                                   &msg);
    }
    
    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_moveUser
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jint nUserID, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        return TTS_MoveUser(reinterpret_cast<TTSInstance*>(lpTTSInstance), nUserID, &chan);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_startServer
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jstring szBindIPAddr, 
     jint nTcpPort, jint nUdpPort, jboolean bEncrypted) {
        return TTS_StartServer(reinterpret_cast<TTSInstance*>(lpTTSInstance), 
                               ttstr(env, szBindIPAddr), 
                               UINT16(nTcpPort), UINT16(nUdpPort), bEncrypted);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_startServerSysID
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jstring szBindIPAddr, 
     jint nTcpPort, jint nUdpPort, jboolean bEncrypted, jstring szSystemID) {
        return TTS_StartServerSysID(reinterpret_cast<TTSInstance*>(lpTTSInstance), 
                                    ttstr(env, szBindIPAddr), 
                                    UINT16(nTcpPort), UINT16(nUdpPort), bEncrypted,
                                    ttstr(env, szSystemID));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_stopServer
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance) {
        return TTS_StopServer(reinterpret_cast<TTSInstance*>(lpTTSInstance));
    }

}
