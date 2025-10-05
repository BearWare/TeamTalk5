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

#include "ttconvert-jni.h"

#include <cassert>
#include <map>
#include <mutex>

#include <TeamTalkSrv.h>

using jenv_t = std::map<TTSInstance*, JNIEnv*>;
using refs_t = std::map<TTSInstance*, jobject>;
static jenv_t envs;
static refs_t command_refs, logger_refs;

static std::mutex ttsinstmutex;
static std::map<jint, TTSInstance*> ttsinstances;

static void AddTTSInstance(JNIEnv* env, jobject thiz, TTSInstance* ttsinst)
{
    auto hash = hashCode(env, thiz);

    std::lock_guard<std::mutex> const g(ttsinstmutex);
    ttsinstances[hash] = ttsinst;
}

static TTSInstance* RemoveTTSInstance(JNIEnv* env, jobject thiz)
{
    auto hash = hashCode(env, thiz);

    std::lock_guard<std::mutex> const g(ttsinstmutex);
    TTSInstance* ttsinst = ttsinstances[hash];
    ttsinstances.erase(hash);
    return ttsinst;
}

static TTSInstance* GetTTSInstance(JNIEnv* env, jobject thiz)
{
    auto hash = hashCode(env, thiz);
    std::lock_guard<std::mutex> const g(ttsinstmutex);
    return ttsinstances[hash];
}


extern "C" {

    /* Client command callbacks */

    void userLoginCallback(IN TTSInstance* lpTTSInstance,
                           IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, errmsg_obj, user_obj, useraccount_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
        setUserAccount(env, *lpUserAccount, useraccount_obj, J2N);
    }
    void userChangeNicknameCallback(IN TTSInstance* lpTTSInstance,
                                    IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, errmsg_obj, user_obj, newnick_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userChangeStatusCallback(IN TTSInstance* lpTTSInstance,
                                  IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, errmsg_obj, user_obj, nNewStatusMode, newstatusmsg_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userCreateUserAccountCallback(IN TTSInstance* lpTTSInstance,
                                       IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, errmsg_obj, user_obj, useraccount_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userDeleteUserAccountCallback(IN TTSInstance* lpTTSInstance,
                                       IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, errmsg_obj, user_obj, username_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userAddServerBanCallback(IN TTSInstance* lpTTSInstance,
                                  IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, errmsg_obj, banner_obj, banee_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userAddServerBanIPAddressCallback(IN TTSInstance* lpTTSInstance,
                                           IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, errmsg_obj, banner_obj, ipaddr_str);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }
    void userDeleteServerBanCallback(IN TTSInstance* lpTTSInstance,
                                     IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, errmsg_obj, user_obj, ipaddr_str);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
    }

    /* Log events */

    void logUserConnectedCallback(IN TTSInstance* lpTTSInstance,
                                  IN VOID* /*lpUserData*/, IN const User* lpUser) {

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
        env->CallVoidMethod(cb_obj, method, user_obj);
    }
    void logUserLoggedInCallback(IN TTSInstance* lpTTSInstance,
                                 IN VOID* /*lpUserData*/, IN const User* lpUser) {
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
        env->CallVoidMethod(cb_obj, method, user_obj);
    }
    void logUserLoggedOutCallback(IN TTSInstance* lpTTSInstance,
                                  IN VOID* /*lpUserData*/, IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userLoggedOut",
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        env->CallVoidMethod(cb_obj, method, user_obj);
    }
    void logUserDisconnectedCallback(IN TTSInstance* lpTTSInstance,
                                     IN VOID* /*lpUserData*/, IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userDisconnected",
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        env->CallVoidMethod(cb_obj, method, user_obj);
    }
    void logUserTimedoutCallback(IN TTSInstance* lpTTSInstance,
                                 IN VOID* /*lpUserData*/, IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = newUser(env, lpUser);
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userTimedout",
                                            "(Ldk/bearware/User;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        env->CallVoidMethod(cb_obj, method, user_obj);
    }
    void logUserKickedCallback(IN TTSInstance* lpTTSInstance,
                               IN VOID* /*lpUserData*/, IN const User* lpKicker,
                               IN const User* lpKickee, IN const Channel* lpChannel) {

        JNIEnv* env = envs[lpTTSInstance];

        jobject kicker_obj = (lpKicker != nullptr)? newUser(env, lpKicker) : nullptr;
        assert(lpKickee);
        jobject kickee_obj = newUser(env, lpKickee);
        jobject channel_obj = (lpChannel != nullptr)? newChannel(env, lpChannel) : nullptr;
        jclass cls = env->FindClass("dk/bearware/ServerLogger");
        assert(cls);
        jmethodID method = env->GetMethodID(cls, "userKicked",
                                            "(Ldk/bearware/User;Ldk/bearware/User;Ldk/bearware/Channel;)V");
        assert(method);

        jobject cb_obj = logger_refs[lpTTSInstance];
        assert(cb_obj);
        env->CallVoidMethod(cb_obj, method, kicker_obj, kickee_obj, channel_obj);
    }
    void logUserBannedCallback(IN TTSInstance* lpTTSInstance,
                               IN VOID* /*lpUserData*/, IN const User* lpBanner,
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
        env->CallVoidMethod(cb_obj, method, banner_obj, banee_obj, channel_obj);
    }
    void logUserUnbannedCallback(IN TTSInstance* lpTTSInstance,
                                 IN VOID* /*lpUserData*/, IN const User* lpUnbanner,
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
        env->CallVoidMethod(cb_obj, method, banner_obj, ipaddr_str);
    }
    void logUserUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                IN VOID* /*lpUserData*/, IN const User* lpUser) {
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
        env->CallVoidMethod(cb_obj, method, user_obj);
    }
    void logUserJoinedChannelCallback(IN TTSInstance* lpTTSInstance,
                                      IN VOID* /*lpUserData*/, IN const User* lpUser,
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
        env->CallVoidMethod(cb_obj, method, user_obj, chan_obj);
    }
    void logUserLeftChannelCallback(IN TTSInstance* lpTTSInstance,
                                    IN VOID* /*lpUserData*/, IN const User* lpUser,
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
        env->CallVoidMethod(cb_obj, method, user_obj, chan_obj);
    }
    void logUserMovedCallback(IN TTSInstance* lpTTSInstance,
                              IN VOID* /*lpUserData*/, IN const User* lpMover,
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
        env->CallVoidMethod(cb_obj, method, mover_obj, movee_obj);
    }
    void logUserTextMessageCallback(IN TTSInstance* lpTTSInstance,
                                    IN VOID* /*lpUserData*/, IN const User* lpUser,
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
        env->CallVoidMethod(cb_obj, method, user_obj, tm_obj);
    }
    void logChannelCreatedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* /*lpUserData*/, IN const Channel* lpChannel,
                                   IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = (lpUser != nullptr) ? newUser(env, lpUser) : nullptr;
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
        env->CallVoidMethod(cb_obj, method, chan_obj, user_obj);
    }
    void logChannelUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* /*lpUserData*/, IN const Channel* lpChannel,
                                   IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = (lpUser != nullptr) ? newUser(env, lpUser) : nullptr;
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
        env->CallVoidMethod(cb_obj, method, chan_obj, user_obj);
    }
    void logChannelRemovedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* /*lpUserData*/, IN const Channel* lpChannel,
                                   IN const User* lpUser) {
        JNIEnv* env = envs[lpTTSInstance];

        jobject user_obj = (lpUser != nullptr) ? newUser(env, lpUser) : nullptr;
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
        env->CallVoidMethod(cb_obj, method, chan_obj, user_obj);
    }
    void logFileUploadedCallback(IN TTSInstance* lpTTSInstance,
                                 IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, file_obj, user_obj);
    }
    void logFileDownloadedCallback(IN TTSInstance* lpTTSInstance,
                                   IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, file_obj, user_obj);
    }
    void logFileDeletedCallback(IN TTSInstance* lpTTSInstance,
                                IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, file_obj, user_obj);
    }
    void logServerUpdatedCallback(IN TTSInstance* lpTTSInstance,
                                  IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, sp_obj, user_obj);
    }
    void logSaveServerConfigCallback(IN TTSInstance* lpTTSInstance,
                                     IN VOID* /*lpUserData*/,
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
        env->CallVoidMethod(cb_obj, method, user_obj);
    }

    JNIEXPORT jstring JNICALL Java_dk_bearware_TeamTalkSrv_getVersion(JNIEnv* env,
                                                                      jclass /*unused*/)
    {
        const TTCHAR* ttv = TT_GetVersion();
        return NEW_JSTRING(env, ttv);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_setLicenseInformation(JNIEnv* env,
                                                                                   jclass /*unused*/,
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
        AddTTSInstance(env, thiz, ttsInst);
        envs[ttsInst] = env;

        return jlong(ttsInst);
    }

    JNIEXPORT void JNICALL Java_dk_bearware_TeamTalkSrv_closeTeamTalk
    (JNIEnv *env, jobject thiz) {
        TTSInstance* inst = RemoveTTSInstance(env, thiz);

        TTS_CloseTeamTalk(inst);

        if(command_refs[inst] != nullptr)
            env->DeleteGlobalRef(command_refs[inst]);
        if(logger_refs[inst] != nullptr)
            env->DeleteGlobalRef(logger_refs[inst]);

        command_refs.erase(inst);
        logger_refs.erase(inst);
        envs.erase(inst);
    }

    JNIEXPORT void JNICALL Java_dk_bearware_TeamTalkSrv_registerServerCallback
    (JNIEnv *env, jobject thiz, jobject servercallback) {

        TTSInstance* inst = GetTTSInstance(env, thiz);

        command_refs[inst] = env->NewGlobalRef(servercallback);

        TTS_RegisterUserLoginCallback(inst, userLoginCallback,
                                      servercallback, 1);
        TTS_RegisterUserChangeNicknameCallback(inst, userChangeNicknameCallback,
                                               servercallback, 1);
        TTS_RegisterUserChangeStatusCallback(inst, userChangeStatusCallback,
                                             servercallback, 1);
        TTS_RegisterUserCreateUserAccountCallback(inst, userCreateUserAccountCallback,
                                                  servercallback, 1);
        TTS_RegisterUserDeleteUserAccountCallback(inst, userDeleteUserAccountCallback,
                                                  servercallback, 1);
        TTS_RegisterUserAddServerBanCallback(inst, userAddServerBanCallback,
                                             servercallback, 1);
        TTS_RegisterUserAddServerBanIPAddressCallback(inst, userAddServerBanIPAddressCallback,
                                                      servercallback, 1);
        TTS_RegisterUserDeleteServerBanCallback(inst, userDeleteServerBanCallback,
                                                servercallback, 1);
    }

    JNIEXPORT void JNICALL Java_dk_bearware_TeamTalkSrv_registerServerLogger
    (JNIEnv *env, jobject thiz, jobject serverlogger) {

        TTSInstance* inst = GetTTSInstance(env, thiz);

        logger_refs[inst] = env->NewGlobalRef(serverlogger);

        TTS_RegisterUserConnectedCallback(inst, logUserConnectedCallback, nullptr, 1);
        TTS_RegisterUserLoggedInCallback(inst, logUserLoggedInCallback, nullptr, 1);
        TTS_RegisterUserLoggedOutCallback(inst, logUserLoggedOutCallback, nullptr, 1);
        TTS_RegisterUserDisconnectedCallback(inst, logUserDisconnectedCallback, nullptr, 1);
        TTS_RegisterUserTimedoutCallback(inst, logUserTimedoutCallback, nullptr, 1);
        TTS_RegisterUserKickedCallback(inst, logUserKickedCallback, nullptr, 1);
        TTS_RegisterUserBannedCallback(inst, logUserBannedCallback, nullptr, 1);
        TTS_RegisterUserUnbannedCallback(inst, logUserUnbannedCallback, nullptr, 1);
        TTS_RegisterUserUpdatedCallback(inst, logUserUpdatedCallback, nullptr, 1);
        TTS_RegisterUserJoinedChannelCallback(inst, logUserJoinedChannelCallback, nullptr, 1);
        TTS_RegisterUserLeftChannelCallback(inst, logUserLeftChannelCallback, nullptr, 1);
        TTS_RegisterUserMovedCallback(inst, logUserMovedCallback, nullptr, 1);
        TTS_RegisterUserTextMessageCallback(inst, logUserTextMessageCallback, nullptr, 1);
        TTS_RegisterChannelCreatedCallback(inst, logChannelCreatedCallback, nullptr, 1);
        TTS_RegisterChannelUpdatedCallback(inst, logChannelUpdatedCallback, nullptr, 1);
        TTS_RegisterChannelRemovedCallback(inst, logChannelRemovedCallback, nullptr, 1);
        TTS_RegisterFileUploadedCallback(inst, logFileUploadedCallback, nullptr, 1);
        TTS_RegisterFileDownloadedCallback(inst, logFileDownloadedCallback, nullptr, 1);
        TTS_RegisterFileDeletedCallback(inst, logFileDeletedCallback, nullptr, 1);
        TTS_RegisterServerUpdatedCallback(inst, logServerUpdatedCallback, nullptr, 1);
        TTS_RegisterSaveServerConfigCallback(inst, logSaveServerConfigCallback, nullptr, 1);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_setEncryptionContext
    (JNIEnv *env, jobject thiz, jstring szCertificateFile, jstring szPrivateKeyFile) {
        return TTS_SetEncryptionContext(GetTTSInstance(env, thiz), ttstr(env, szCertificateFile),
                                        ttstr(env, szPrivateKeyFile));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_setEncryptionContextEx
    (JNIEnv *env, jobject thiz, jobject lpEncryptionContext) {

        THROW_NULLEX(env, lpEncryptionContext, false);

        EncryptionContext context = {};
        setEncryptionContext(env, context, lpEncryptionContext, J2N);

        return TTS_SetEncryptionContextEx(GetTTSInstance(env, thiz),
                                          &context);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_runEventLoop
    (JNIEnv *env, jobject thiz, jint pnWaitMs) {

        envs[GetTTSInstance(env, thiz)] = env;

        INT32 _pnWaitMs = pnWaitMs;

        return TTS_RunEventLoop(GetTTSInstance(env, thiz), &_pnWaitMs);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_setChannelFilesRoot
    (JNIEnv *env, jobject thiz, jstring szFilesRoot,
     jlong nMaxDiskUsage, jlong nDefaultChannelQuota) {

        return TTS_SetChannelFilesRoot(GetTTSInstance(env, thiz),
                                       ttstr(env, szFilesRoot), nMaxDiskUsage, nDefaultChannelQuota);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_updateServer
    (JNIEnv *env, jobject thiz, jobject lpServerProperties) {

        ServerProperties srvprop;
        setServerProperties(env, srvprop, lpServerProperties, J2N);
        return TTS_UpdateServer(GetTTSInstance(env, thiz), &srvprop);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_makeChannel
    (JNIEnv *env, jobject thiz, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);

        return TTS_MakeChannel(GetTTSInstance(env, thiz), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_updateChannel
    (JNIEnv *env, jobject thiz, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);

        return TTS_UpdateChannel(GetTTSInstance(env, thiz), &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_removeChannel
    (JNIEnv *env, jobject thiz, jint nChannelID) {
        return TTS_RemoveChannel(GetTTSInstance(env, thiz), nChannelID);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_addFileToChannel
        (JNIEnv *env, jobject thiz, jstring szLocalFilePath, jobject lpRemoteFile) {
        RemoteFile rmfile;
        setRemoteFile(env, rmfile, lpRemoteFile, J2N);

        return TTS_AddFileToChannel(GetTTSInstance(env, thiz),
                                    ttstr(env, szLocalFilePath), &rmfile);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_removeFileFromChannel
        (JNIEnv *env, jobject thiz, jobject lpRemoteFile) {

        RemoteFile rmfile;
        setRemoteFile(env, rmfile, lpRemoteFile, J2N);

        return TTS_RemoveFileFromChannel(GetTTSInstance(env, thiz),
                                         &rmfile);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_sendTextMessage
        (JNIEnv *env, jobject thiz, jobject lpTextMessage) {

        TextMessage msg;
        setTextMessage(env, msg, lpTextMessage, J2N);

        return TTS_SendTextMessage(GetTTSInstance(env, thiz),
                                   &msg);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_moveUser
    (JNIEnv *env, jobject thiz, jint nUserID, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        return TTS_MoveUser(GetTTSInstance(env, thiz), nUserID, &chan);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_startServer
    (JNIEnv *env, jobject thiz, jstring szBindIPAddr,
     jint nTcpPort, jint nUdpPort, jboolean bEncrypted) {
        return TTS_StartServer(GetTTSInstance(env, thiz),
                               ttstr(env, szBindIPAddr),
                               UINT16(nTcpPort), UINT16(nUdpPort), bEncrypted);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_startServerSysID
    (JNIEnv *env, jobject thiz, jstring szBindIPAddr,
     jint nTcpPort, jint nUdpPort, jboolean bEncrypted, jstring szSystemID) {
        return TTS_StartServerSysID(GetTTSInstance(env, thiz),
                                    ttstr(env, szBindIPAddr),
                                    UINT16(nTcpPort), UINT16(nUdpPort), bEncrypted,
                                    ttstr(env, szSystemID));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_stopServer
    (JNIEnv *env, jobject thiz) {
        return TTS_StopServer(GetTTSInstance(env, thiz));
    }

}
