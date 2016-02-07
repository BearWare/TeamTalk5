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


typedef map<jobject, TTSInstance*> ttsinst_t;

ttsinst_t instances;

extern "C" {


    JNIEXPORT jlong JNICALL Java_dk_bearware_TeamTalkSrv_initTeamTalk
    (JNIEnv *env, jobject thiz) {
        TTSInstance* ttsInst = TTS_InitTeamTalk();
        instances[thiz] = ttsInst;

        return jlong(ttsInst);
    }

    JNIEXPORT void JNICALL Java_dk_bearware_TeamTalkSrv_closeTeamTalk
    (JNIEnv *env, jobject thiz) {
        TTS_CloseTeamTalk(instances[thiz]);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_setEncryptionContext
    (JNIEnv *env, jobject thiz, jstring szCertificateFile, jstring szPrivateKeyFile) {
        return TTS_SetEncryptionContext(instances[thiz], ttstr(env, szCertificateFile),
                                        ttstr(env, szPrivateKeyFile));
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_runEventLoop
    (JNIEnv *env, jobject thiz, jint pnWaitMs) {
        return TTS_RunEventLoop(instances[thiz], &pnWaitMs);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_updateServer
    (JNIEnv *env, jobject thiz, jobject lpServerProperties) {

        ServerProperties srvprop;
        setServerProperties(env, srvprop, lpServerProperties, J2N);
        return TTS_UpdateServer(instances[thiz], &srvprop);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_makeChannel
    (JNIEnv *env, jobject thiz, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        
        return TTS_MakeChannel(instances[thiz], &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_updateChannel
    (JNIEnv *env, jobject thiz, jobject lpChannel) {
        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        
        return TTS_UpdateChannel(instances[thiz], &chan);
    }

    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_removeChannel
    (JNIEnv *env, jobject thiz, jint nChannelID) {
        return TTS_RemoveChannel(instances[thiz], nChannelID);
    }
    
    JNIEXPORT jint JNICALL Java_dk_bearware_TeamTalkSrv_moveUser
    (JNIEnv *env, jobject thiz, jint nUserID, jobject lpChannel) {

        Channel chan;
        setChannel(env, chan, lpChannel, J2N);
        return TTS_MoveUser(instances[thiz], nUserID, &chan);
    }

    JNIEXPORT jboolean JNICALL Java_dk_bearware_TeamTalkSrv_startServer
    (JNIEnv *env, jobject thiz, jstring szBindIPAddr, jint nTcpPort, jint nUdpPort, jboolean bEncrypted) {
        return TTS_StartServer(instances[thiz], ttstr(env, szBindIPAddr), 
                               nTcpPort, nUdpPort, bEncrypted);
    }

}
