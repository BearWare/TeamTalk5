/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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
refs_t refs;

extern "C" {

    void userLoginCallback(IN TTSInstance* lpTTSInstance, 
                           IN VOID* lpUserData,
                           OUT ClientErrorMsg* lpClientErrorMsg,
                           IN const User* lpUser, 
                           IN OUT UserAccount* lpUserAccount) {
        
        JNIEnv* env = envs[lpTTSInstance];

        jclass cls = env->FindClass("dk/bearware/ClientErrorMsg");
        jobject errmsg_obj = newObject(env, cls);
        assert(errmsg_obj);
        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, N2J);
        
        cls = env->FindClass("dk/bearware/User");
        jobject user_obj = newObject(env, cls);
        assert(user_obj);
        setUser(env, *lpUser, user_obj);

        cls = env->FindClass("dk/bearware/UserAccount");
        jobject useraccount_obj = newObject(env, cls);
        assert(useraccount_obj);
        setUserAccount(env, *lpUserAccount, useraccount_obj, N2J);

        cls = env->FindClass("dk/bearware/ServerCallback");
        jmethodID method = env->GetMethodID(cls, "userLogin", 
                                            "(Ldk/bearware/ClientErrorMsg;Ldk/bearware/User;Ldk/bearware/UserAccount;)V");
        assert(method);

        jobject cb_obj = refs[lpTTSInstance];
        env->CallObjectMethod(cb_obj, method, errmsg_obj, user_obj, useraccount_obj);

        setClientErrorMsg(env, *lpClientErrorMsg, errmsg_obj, J2N);
        setUserAccount(env, *lpUserAccount, useraccount_obj, J2N);
    }

    void userCreateUserAccountCallback(IN TTSInstance* lpTTSInstance, 
                                       IN VOID* lpUserData,
                                       OUT ClientErrorMsg* lpClientErrorMsg,
                                       IN const User* lpUser, 
                                       OUT UserAccount* lpUserAccount) {
    }

    void userDeleteUserAccountCallback(IN TTSInstance* lpTTSInstance, 
                                       IN VOID* lpUserData,
                                       OUT ClientErrorMsg* lpClientErrorMsg,
                                       IN const User* lpUser, 
                                       IN const TTCHAR* szUsername) {
    }

    void userAddServerBanCallback(IN TTSInstance* lpTTSInstance, 
                                  IN VOID* lpUserData,
                                  OUT ClientErrorMsg* lpClientErrorMsg,
                                  IN const User* lpBanner, 
                                  IN const User* lpBanee) {
    }

    void userAddServerBanIPAddressCallback(IN TTSInstance* lpTTSInstance, 
                                           IN VOID* lpUserData,
                                           OUT ClientErrorMsg* lpClientErrorMsg,
                                           IN const User* lpBanner, 
                                           IN const TTCHAR* szIPAddress) {
    }

    void userDeleteServerBanCallback(IN TTSInstance* lpTTSInstance, 
                                     IN VOID* lpUserData,
                                     OUT ClientErrorMsg* lpClientErrorMsg,
                                     IN const User* lpUser, 
                                     IN const TTCHAR* szIPAddress) {
    }

    JNIEXPORT jlong JNICALL Java_dk_bearware_TeamTalkSrv_initTeamTalk
    (JNIEnv *env, jobject thiz) {

        TTSInstance* ttsInst = TTS_InitTeamTalk();

        return jlong(ttsInst);
    }

    JNIEXPORT void JNICALL Java_dk_bearware_TeamTalkSrv_closeTeamTalk
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance) {
        TTSInstance* inst = reinterpret_cast<TTSInstance*>(lpTTSInstance);

        TTS_CloseTeamTalk(inst);
        
        if(refs[inst])
            env->DeleteGlobalRef(refs[inst]);
        refs.erase(inst);
        envs.erase(inst);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_registerServerCallback
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jobject servercallback) {

        TTSInstance* inst = reinterpret_cast<TTSInstance*>(lpTTSInstance);

        refs[inst] = env->NewGlobalRef(servercallback);

        TTS_RegisterUserLoginCallback(inst, userLoginCallback,
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

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_setEncryptionContext
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jstring szCertificateFile, jstring szPrivateKeyFile) {
        return TTS_SetEncryptionContext(reinterpret_cast<TTSInstance*>(lpTTSInstance), ttstr(env, szCertificateFile),
                                        ttstr(env, szPrivateKeyFile));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_runEventLoop
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jint pnWaitMs) {

        envs[reinterpret_cast<TTSInstance*>(lpTTSInstance)] = env;

        return TTS_RunEventLoop(reinterpret_cast<TTSInstance*>(lpTTSInstance), &pnWaitMs);
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
    
    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_moveUser
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jint nUserID, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        return TTS_MoveUser(reinterpret_cast<TTSInstance*>(lpTTSInstance), nUserID, &chan);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_startServer
    (JNIEnv *env, jobject thiz, jlong lpTTSInstance, jstring szBindIPAddr, jint nTcpPort, jint nUdpPort, jboolean bEncrypted) {
        return TTS_StartServer(reinterpret_cast<TTSInstance*>(lpTTSInstance), 
                               ttstr(env, szBindIPAddr), 
                               nTcpPort, nUdpPort, bEncrypted);
    }

}
